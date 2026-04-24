#include "mainwindow.h"
#include <QMenuBar>
#include <QToolBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QStatusBar>
#include <QAction>
#include <QCloseEvent>
#include <QTextCursor>
#include <QTextBlock>
#include <QPainter>
#include <QScrollBar>
#include <sstream>

// ===========================================================================
// CodeEditor
// ===========================================================================
CodeEditor::CodeEditor(QWidget* parent)
    : QWidget(parent)
    , m_textEdit(new QTextEdit(this))
    , m_lineNumberArea(new LineNumberArea(this))
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_lineNumberArea);
    layout->addWidget(m_textEdit);

    setupLineNumberArea();

    connect(m_textEdit->document(), &QTextDocument::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);
    connect(m_textEdit->verticalScrollBar(), &QScrollBar::valueChanged,
            this, [this](int) { m_lineNumberArea->update(); });
    connect(m_textEdit, &QTextEdit::textChanged,
            this, [this]() { m_lineNumberArea->update(); });

    updateLineNumberAreaWidth(0);
}

void CodeEditor::setupLineNumberArea() {
    m_lineNumberArea->setFont(m_textEdit->font());
}

void CodeEditor::applyTheme(bool isDark) {
    m_isDarkMode = isDark;

    QFont font("Courier New", 10);
    font.setStyleHint(QFont::Monospace);
    m_textEdit->setFont(font);
    m_lineNumberArea->setFont(font);

    if (isDark) {
        m_textEdit->setStyleSheet(
            "QTextEdit { background-color:#1e1e1e; color:#d4d4d4; border:none;"
            "  selection-background-color:#264f78; }");
        m_lineNumberArea->setStyleSheet("background-color:#252526; color:#858585;");
    } else {
        m_textEdit->setStyleSheet(
            "QTextEdit { background-color:#ffffff; color:#000000; border:none;"
            "  selection-background-color:#add6ff; }");
        m_lineNumberArea->setStyleSheet("background-color:#f3f3f3; color:#237893;");
    }
    m_lineNumberArea->update();
}

int CodeEditor::lineNumberAreaWidth() {
    int digits = 1;
    int max = qMax(1, m_textEdit->document()->blockCount());
    while (max >= 10) { max /= 10; ++digits; }
    return 10 + m_textEdit->fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

void CodeEditor::updateLineNumberAreaWidth(int) {
    m_lineNumberArea->setFixedWidth(lineNumberAreaWidth());
    m_lineNumberArea->update();
}

void CodeEditor::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(cr.left(), cr.top(),
                                  lineNumberAreaWidth(), cr.height());
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent* event) {
    QPainter painter(m_lineNumberArea);
    if (m_isDarkMode) {
        painter.fillRect(event->rect(), QColor("#252526"));
        painter.setPen(QColor("#858585"));
    } else {
        painter.fillRect(event->rect(), QColor("#f3f3f3"));
        painter.setPen(QColor("#237893"));
    }
    painter.setFont(m_textEdit->font());

    int lineHeight = m_textEdit->fontMetrics().height();
    QTextCursor topCursor = m_textEdit->cursorForPosition({0, 0});
    if (topCursor.isNull()) return;

    QTextBlock block = topCursor.block();
    int blockNumber  = block.blockNumber();

    QTextCursor bs(block);
    bs.movePosition(QTextCursor::StartOfBlock);
    int y = m_textEdit->cursorRect(bs).topLeft().y();

    while (block.isValid() && y < event->rect().bottom() + lineHeight) {
        if (y + lineHeight >= event->rect().top()) {
            painter.drawText(0, y, m_lineNumberArea->width() - 5, lineHeight,
                             Qt::AlignRight | Qt::AlignVCenter,
                             QString::number(blockNumber + 1));
        }
        block = block.next();
        ++blockNumber;
        if (block.isValid()) {
            QTextCursor nb(block);
            nb.movePosition(QTextCursor::StartOfBlock);
            y = m_textEdit->cursorRect(nb).topLeft().y();
        } else {
            y += lineHeight;
        }
    }
}

// ===========================================================================
// MainWindow
// ===========================================================================
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , controller(std::make_unique<JSONController>())
{
    setupUI();
    setupMenuBar();
    setupToolBar();

    // ---- Instantiate sub-controllers ----------------------------------------
    searchCtrl   = std::make_unique<SearchController>(textEditor->textEdit(), this);
    treeCtrl     = std::make_unique<TreeViewController>(treeView, this);
    themeManager = std::make_unique<ThemeManager>(this);

    themeManager->setMainWindow(this);
    themeManager->setTreeView(treeView);
    themeManager->setStatusLabel(statusLabel);
    themeManager->setThemeButton(themeToggleBtn);
    themeManager->setCodeEditor(textEditor);

    setupConnections();

    themeManager->setDark(false);   // apply initial theme

    setWindowTitle("JSON Viewer & Editor");
    resize(1200, 800);
    updateStatusBar("Ready");
}

MainWindow::~MainWindow() {}

// ---------------------------------------------------------------------------
// UI setup
// ---------------------------------------------------------------------------
void MainWindow::setupUI() {
    cursorPosLabel = new QLabel("Line: 1, Column: 1");
    statusBar()->addPermanentWidget(cursorPosLabel);

    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainSplitter = new QSplitter(Qt::Horizontal);

    // Left: editor
    auto* leftContainer = new QWidget();
    auto* leftLayout    = new QVBoxLayout(leftContainer);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);

    auto* editorLabel = new QLabel("JSON Editor");
    editorLabel->setStyleSheet("font-weight:bold; padding:6px 10px; color:#0000FF;");
    editorLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    textEditor = new CodeEditor();
    textEditor->textEdit()->setPlaceholderText(
        "Paste or type JSON here, or open a file...");

    leftLayout->addWidget(editorLabel);
    leftLayout->addWidget(textEditor);

    // Right: tree
    auto* rightContainer = new QWidget();
    auto* rightLayout    = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    auto* treeLabel = new QLabel("Tree View");
    treeLabel->setStyleSheet("font-weight:bold; padding:6px 10px; color:#0000FF;");
    treeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    treeView = new QTreeWidget();
    treeView->setHeaderLabels({"Key", "Value", "Type"});
    treeView->setColumnWidth(0, 200);
    treeView->setColumnWidth(1, 300);
    treeView->setAlternatingRowColors(true);
    treeView->setIndentation(20);
    treeView->setAnimated(true);

    rightLayout->addWidget(treeLabel);
    rightLayout->addWidget(treeView);

    mainSplitter->addWidget(leftContainer);
    mainSplitter->addWidget(rightContainer);
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 1);

    mainLayout->addWidget(mainSplitter);

    statusLabel = new QLabel();
    statusBar()->addPermanentWidget(statusLabel, 1);
}

void MainWindow::setupMenuBar() {
    auto* mb = new QMenuBar(this);
    setMenuBar(mb);

    // File
    auto* fileMenu = mb->addMenu("&File");
    auto* openAct  = new QAction("&Open",     this); openAct->setShortcut(QKeySequence::Open);
    auto* saveAct  = new QAction("&Save",     this); saveAct->setShortcut(QKeySequence::Save);
    auto* saveAsAct= new QAction("Save &As…", this); saveAsAct->setShortcut(QKeySequence::SaveAs);
    auto* exitAct  = new QAction("E&xit",     this); exitAct->setShortcut(QKeySequence::Quit);
    connect(openAct,  &QAction::triggered, this, &MainWindow::onOpenFile);
    connect(saveAct,  &QAction::triggered, this, &MainWindow::onSaveFile);
    connect(saveAsAct,&QAction::triggered, this, &MainWindow::onSaveAsFile);
    connect(exitAct,  &QAction::triggered, this, &MainWindow::close);
    fileMenu->addActions({openAct, saveAct, saveAsAct});
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    // Edit
    auto* editMenu    = mb->addMenu("&Edit");
    auto* formatAct   = new QAction("&Format JSON",   this);
    auto* validateAct = new QAction("&Validate JSON",  this);
    auto* findAct     = new QAction("&Find",           this);
    auto* findNextAct = new QAction("Find &Next",      this);
    auto* findPrevAct = new QAction("Find &Previous",  this);
    formatAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F));
    validateAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));  // V clashes with paste on some
    findAct->setShortcut(QKeySequence::Find);
    findNextAct->setShortcut(QKeySequence::FindNext);
    findPrevAct->setShortcut(QKeySequence::FindPrevious);
    connect(formatAct,   &QAction::triggered, this, &MainWindow::onFormatJSON);
    connect(validateAct, &QAction::triggered, this, &MainWindow::onValidateJSON);
    connect(findAct,     &QAction::triggered, this, [this]{ searchEdit->setFocus(); searchEdit->selectAll(); });
    connect(findNextAct, &QAction::triggered, this, [this]{ searchCtrl->next(); });
    connect(findPrevAct, &QAction::triggered, this, [this]{ searchCtrl->previous(); });
    editMenu->addActions({formatAct, validateAct});
    editMenu->addSeparator();
    editMenu->addActions({findAct, findNextAct, findPrevAct});

    // View
    auto* viewMenu   = mb->addMenu("&View");
    auto* themeAct   = new QAction("Toggle &Theme", this);
    themeAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
    connect(themeAct, &QAction::triggered, this, [this]{ themeManager->toggle(); });
    viewMenu->addAction(themeAct);
}

void MainWindow::setupToolBar() {
    auto* tb = addToolBar("Main Toolbar");
    tb->setMovable(false);
    tb->setIconSize({16, 16});
    tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    auto* openAct     = new QAction(QIcon(), "Open",     this);
    auto* saveAct     = new QAction(QIcon(), "Save",     this);
    auto* formatAct   = new QAction(QIcon(), "Format",   this);
    auto* validateAct = new QAction(QIcon(), "Validate", this);
    connect(openAct,     &QAction::triggered, this, &MainWindow::onOpenFile);
    connect(saveAct,     &QAction::triggered, this, &MainWindow::onSaveFile);
    connect(formatAct,   &QAction::triggered, this, &MainWindow::onFormatJSON);
    connect(validateAct, &QAction::triggered, this, &MainWindow::onValidateJSON);
    tb->addActions({openAct, saveAct});
    tb->addSeparator();
    tb->addActions({formatAct, validateAct});
    tb->addSeparator();

    tb->addWidget(new QLabel(" Mode: "));
    parserModeCombo = new QComboBox();
    parserModeCombo->addItem("Standard", QVariant::fromValue(
                                             static_cast<int>(JSONParserFactory::ParserType::STANDARD)));
    parserModeCombo->addItem("Strict",   QVariant::fromValue(
                                           static_cast<int>(JSONParserFactory::ParserType::STRICT_MODE)));
    parserModeCombo->addItem("Relaxed",  QVariant::fromValue(
                                            static_cast<int>(JSONParserFactory::ParserType::RELAXED)));
    parserModeCombo->setFixedWidth(100);
    parserModeCombo->setCursor(Qt::PointingHandCursor);
    tb->addWidget(parserModeCombo);
    tb->addSeparator();

    tb->addWidget(new QLabel(" Search: "));
    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Search in JSON…");
    searchEdit->setFixedWidth(180);
    searchEdit->setClearButtonEnabled(true);
    tb->addWidget(searchEdit);

    auto* prevBtn = new QPushButton("◀");
    prevBtn->setFixedSize(28, 28);
    prevBtn->setToolTip("Previous Result (Shift+F3)");
    prevBtn->setCursor(Qt::PointingHandCursor);
    tb->addWidget(prevBtn);

    auto* nextBtn = new QPushButton("▶");
    nextBtn->setFixedSize(28, 28);
    nextBtn->setToolTip("Next Result (F3)");
    nextBtn->setCursor(Qt::PointingHandCursor);
    tb->addWidget(nextBtn);

    connect(prevBtn, &QPushButton::clicked, this, [this]{ searchCtrl->previous(); });
    connect(nextBtn, &QPushButton::clicked, this, [this]{ searchCtrl->next(); });

    tb->addSeparator();

    themeToggleBtn = new QPushButton("🌙");
    themeToggleBtn->setFixedSize(32, 28);
    themeToggleBtn->setToolTip("Toggle Dark/Light Theme");
    themeToggleBtn->setCursor(Qt::PointingHandCursor);
    tb->addWidget(themeToggleBtn);
}

void MainWindow::setupConnections() {
    // Text changes → mark modified, refresh search
    connect(textEditor->textEdit(), &QTextEdit::textChanged, this, &MainWindow::onTextChanged);

    // Parser mode combo
    connect(parserModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onParserModeChanged);

    // Cursor position display
    connect(textEditor->textEdit(), &QTextEdit::cursorPositionChanged,
            this, &MainWindow::updateCursorPosition);

    // Search – delegate entirely to SearchController
    connect(searchEdit, &QLineEdit::textChanged,
            this, [this](const QString& text){ searchCtrl->search(text); });
    connect(searchEdit, &QLineEdit::returnPressed,
            this, [this]{ searchCtrl->next(); });
    connect(searchCtrl.get(), &SearchController::statusChanged,
            this, [this](const QString& msg){ updateStatusBar(msg); });

    // Tree – delegate to TreeViewController
    connect(treeCtrl.get(), &TreeViewController::itemSelected,
            this, [this](const QString& msg){ updateStatusBar(msg); });

    // Theme toggle button
    connect(themeToggleBtn, &QPushButton::clicked,
            this, [this]{ themeManager->toggle(); });

    // Theme change → update search highlight colours
    connect(themeManager.get(), &ThemeManager::themeChanged,
            this, [this](bool dark){ searchCtrl->setDarkMode(dark); });
}

// ---------------------------------------------------------------------------
// File operations
// ---------------------------------------------------------------------------
void MainWindow::onOpenFile() {
    if (!maybeSave()) return;

    QString fileName = QFileDialog::getOpenFileName(
        this, "Open JSON File", {},
        "JSON Files (*.json);;All Files (*)");
    if (fileName.isEmpty()) return;

    auto result = controller->openFile(fileName.toStdString(),
                                       controller->getParserMode());
    if (result) {
        std::string formatted = controller->getDocument()
        ? [&]{ std::ostringstream oss;
                   controller->getDocument()->print(oss, 0);
                   return oss.str(); }()
        : std::string{};

        textEditor->textEdit()->blockSignals(true);
        textEditor->textEdit()->setPlainText(QString::fromStdString(formatted));
        textEditor->textEdit()->blockSignals(false);

        treeCtrl->populate(controller->getDocument());
        updateWindowTitle();
        updateStatusBar(QString::fromStdString(result.message));

        if (!result.warnings.empty()) {
            QString w = "Warnings:\n";
            for (auto& s : result.warnings) w += QString::fromStdString(s) + "\n";
            QMessageBox::warning(this, "Parse Warnings", w);
        }
    } else {
        // result.error carries line/column directly – no regex needed
        QString msg = QString::fromStdString(result.message);
        if (result.error.line > 0)
            msg += QString("\n(line %1, column %2)")
                       .arg(result.error.line).arg(result.error.column);
        QMessageBox::critical(this, "Error Opening File", msg);
        updateStatusBar(msg, true);
    }
}

void MainWindow::onSaveFile() {
    // Parse current text first to ensure the model is up to date
    auto parseResult = controller->parseText(
        textEditor->textEdit()->toPlainText().toStdString(),
        controller->getParserMode());

    if (!parseResult) {
        QString msg = QString::fromStdString(parseResult.message);
        if (parseResult.error.line > 0)
            msg += QString("\n(line %1, column %2)")
                       .arg(parseResult.error.line).arg(parseResult.error.column);
        QMessageBox::warning(this, "Cannot Save", "Cannot save invalid JSON:\n" + msg);
        return;
    }

    auto result = controller->saveFile();
    if (!result && result.message.find("No file path") != std::string::npos) {
        onSaveAsFile();
        return;
    }

    if (result) {
        updateWindowTitle();
        updateStatusBar(QString::fromStdString(result.message));
    } else {
        QMessageBox::critical(this, "Error Saving File",
                              QString::fromStdString(result.message));
    }
}

void MainWindow::onSaveAsFile() {
    QString fileName = QFileDialog::getSaveFileName(
        this, "Save JSON File", {},
        "JSON Files (*.json);;All Files (*)");
    if (fileName.isEmpty()) return;

    auto parseResult = controller->parseText(
        textEditor->textEdit()->toPlainText().toStdString(),
        controller->getParserMode());
    if (!parseResult) {
        QMessageBox::warning(this, "Cannot Save",
                             "Cannot save invalid JSON:\n" +
                                 QString::fromStdString(parseResult.message));
        return;
    }

    auto result = controller->saveFileAs(fileName.toStdString());
    if (result) {
        updateWindowTitle();
        updateStatusBar(QString::fromStdString(result.message));
    } else {
        QMessageBox::critical(this, "Error Saving File",
                              QString::fromStdString(result.message));
    }
}

// ---------------------------------------------------------------------------
// Edit operations
// ---------------------------------------------------------------------------
void MainWindow::onFormatJSON() {
    std::string jsonText = textEditor->textEdit()->toPlainText().toStdString();

    auto parseResult = controller->parseText(jsonText, controller->getParserMode());
    if (!parseResult) {
        QString msg = QString::fromStdString(parseResult.message);
        if (parseResult.error.line > 0)
            msg += QString("\n(line %1, column %2)")
                       .arg(parseResult.error.line).arg(parseResult.error.column);
        QMessageBox::warning(this, "Format Error", msg);
        updateStatusBar(msg, true);
        return;
    }

    std::ostringstream oss;
    controller->getDocument()->print(oss, 0);
    QString formatted = QString::fromStdString(oss.str());

    if (formatted == textEditor->textEdit()->toPlainText()) {
        updateStatusBar("JSON is already formatted");
        return;
    }

    textEditor->textEdit()->blockSignals(true);
    textEditor->textEdit()->setPlainText(formatted);
    textEditor->textEdit()->blockSignals(false);

    treeCtrl->populate(controller->getDocument());
    updateStatusBar("JSON formatted successfully");

    if (!parseResult.warnings.empty()) {
        QString w = "Formatting completed with warnings:\n";
        for (auto& s : parseResult.warnings) w += QString::fromStdString(s) + "\n";
        QMessageBox::warning(this, "Format Warnings", w);
    }
}

void MainWindow::onValidateJSON() {
    auto parseResult = controller->parseText(
        textEditor->textEdit()->toPlainText().toStdString(),
        controller->getParserMode());

    if (!parseResult) {
        QString msg = QString::fromStdString(parseResult.message);
        if (parseResult.error.line > 0)
            msg += QString("\n(line %1, column %2)")
                       .arg(parseResult.error.line).arg(parseResult.error.column);
        QMessageBox::warning(this, "Validation Failed", msg);
        updateStatusBar(msg, true);
        return;
    }

    // Run schema validation if one is active
    QString msg = "JSON is valid!";
    if (controller->hasActiveSchema()) {
        // validateWithSchema uses the already-parsed document
        auto schemaResult = controller->validateJSON(); // structural check
        // schema check via active schema happens automatically via appendSchemaWarnings
        if (!parseResult.warnings.empty()) {
            msg += "\n\nSchema warnings:\n";
            for (auto& w : parseResult.warnings)
                msg += QString::fromStdString(w) + "\n";
        }
    } else if (!parseResult.warnings.empty()) {
        msg += "\n\nWarnings:\n";
        for (auto& w : parseResult.warnings)
            msg += QString::fromStdString(w) + "\n";
    }

    QMessageBox::information(this, "Validation", msg);
    updateStatusBar("JSON validation successful");
}

// ---------------------------------------------------------------------------
// UI helpers
// ---------------------------------------------------------------------------
void MainWindow::onParserModeChanged(int index) {
    auto mode = static_cast<JSONParserFactory::ParserType>(
        parserModeCombo->itemData(index).toInt());
    controller->setParserMode(mode);
    updateStatusBar("Parser mode: " + parserModeCombo->currentText());
}

void MainWindow::onTextChanged() {
    controller->markAsModified();
    setWindowModified(true);
    // Refresh search highlights against the new text
    if (searchCtrl->hasResults())
        searchCtrl->onDocumentChanged();
}

void MainWindow::updateCursorPosition() {
    QTextCursor c = textEditor->textEdit()->textCursor();
    cursorPosLabel->setText(
        QString("Line: %1, Column: %2")
            .arg(c.blockNumber() + 1)
            .arg(c.positionInBlock() + 1));
}

void MainWindow::updateStatusBar(const QString& message, bool /*isError*/) {
    statusLabel->setText(message);
}

void MainWindow::updateWindowTitle() {
    QString title = "JSON Viewer & Editor";
    auto fp = controller->getCurrentFilePath();
    if (!fp.empty()) title += " - " + QString::fromStdString(fp);
    setWindowTitle(title);
    setWindowModified(controller->hasUnsavedChanges());
}

bool MainWindow::maybeSave() {
    if (!controller->hasUnsavedChanges()) return true;

    auto ret = QMessageBox::warning(
        this, "Unsaved Changes",
        "The document has been modified.\nDo you want to save your changes?",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save) { onSaveFile(); return !controller->hasUnsavedChanges(); }
    if (ret == QMessageBox::Cancel) return false;
    return true;
}

void MainWindow::closeEvent(QCloseEvent* event) {
    maybeSave() ? event->accept() : event->ignore();
}
