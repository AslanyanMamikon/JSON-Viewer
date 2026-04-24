#ifndef COMMENT_PRESERVATION_H_
#define COMMENT_PRESERVATION_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

// Represents a comment with its position information
struct Comment {
    enum class Type {
        LINE,       // // comment
        BLOCK       // /* comment */
    };

    Type type;
    std::string text;
    size_t line;
    size_t column;
    bool isTrailing;

    Comment(Type t, const std::string& txt, size_t ln, size_t col, bool trailing = false)
        : type(t), text(txt), line(ln), column(col), isTrailing(trailing) {}
};

// Stores comments associated with JSON paths
// Path examples: "root", "root.name", "root.users[0]", "root.users[0].name"
class CommentStore {
private:
    // Maps JSON path to comments before/after that element
    std::unordered_map<std::string, std::vector<Comment>> beforeComments;
    std::unordered_map<std::string, std::vector<Comment>> trailingComments;
    std::unordered_map<std::string, std::vector<Comment>> afterComments;

    // Comments that appear before the root element
    std::vector<Comment> headerComments;

    // Comments that appear after the entire JSON
    std::vector<Comment> footerComments;

public:
    void addBeforeComment(const std::string& path, const Comment& comment) {
        beforeComments[path].push_back(comment);
    }

    void addTrailingComment(const std::string& path, const Comment& comment) {
        trailingComments[path].push_back(comment);
    }

    void addAfterComment(const std::string& path, const Comment& comment) {
        afterComments[path].push_back(comment);
    }

    void addHeaderComment(const Comment& comment) {
        headerComments.push_back(comment);
    }

    void addFooterComment(const Comment& comment) {
        footerComments.push_back(comment);
    }

    const std::vector<Comment>& getBeforeComments(const std::string& path) const {
        static std::vector<Comment> empty;
        auto it = beforeComments.find(path);
        return it != beforeComments.end() ? it->second : empty;
    }

    const std::vector<Comment>& getTrailingComments(const std::string& path) const {
        static std::vector<Comment> empty;
        auto it = trailingComments.find(path);
        return it != trailingComments.end() ? it->second : empty;
    }

    const std::vector<Comment>& getAfterComments(const std::string& path) const {
        static std::vector<Comment> empty;
        auto it = afterComments.find(path);
        return it != afterComments.end() ? it->second : empty;
    }

    const std::vector<Comment>& getHeaderComments() const {
        return headerComments;
    }

    const std::vector<Comment>& getFooterComments() const {
        return footerComments;
    }

    void clear() {
        beforeComments.clear();
        trailingComments.clear();
        afterComments.clear();
        headerComments.clear();
        footerComments.clear();
    }

    bool isEmpty() const {
        return beforeComments.empty() && trailingComments.empty() &&
               afterComments.empty() && headerComments.empty() &&
               footerComments.empty();
    }
};

#endif // COMMENT_PRESERVATION_H_
