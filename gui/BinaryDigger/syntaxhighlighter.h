#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QJsonObject>
#include <QHash>
#include <QTextCharFormat>

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

 public:
     SyntaxHighlighter(QTextDocument *parent = 0);

     void initSyntax(const QString& syntaxFilePath);
     void cleanSyntax();

 protected:
     void highlightBlock(const QString& text);

 private:
     typedef std::map<QString, QFont::Weight> WeightsMap;
     typedef std::map<QString, Qt::GlobalColor> GlobalColorMap;

     WeightsMap weightsMap;
     GlobalColorMap globalColorMap;

     void initFormatReference(const QJsonObject& templ, QTextCharFormat& format);
     QFont::Weight getWeight(const QJsonObject& templ, QFont::Weight def) const;
     Qt::GlobalColor getForeground(const QJsonObject& templ, Qt::GlobalColor def) const;
     bool getItalic(const QJsonObject& templ, bool def) const;

     void setFormatObject(const QJsonObject& templ, QTextCharFormat& format);

     void initFormatTemplate(const QString& name, const QJsonObject& templ);
     void initRegexpsTemplate(const QString& name, const QJsonObject& templ);
     void initKeywordsTemplate(const QString& name, const QJsonObject& templ);
     void initCommentsTemplate(const QString& name, const QJsonObject& templ);

     int findClosestComment(const QString& text, int startIndex, int& ruleIndex);
     void highlightComments(const QString& text);

     typedef std::map<QString, QTextCharFormat> FormatsMap;
     FormatsMap formatsMap;

     struct HighlightingRule
     {
         QRegExp         pattern;
         QTextCharFormat format;
     };

     QVector<HighlightingRule> highlightingRules;

     struct CommentingRule
     {
         QRegExp         beginRegexp;
         QRegExp         endRegexp;
         QTextCharFormat format;
     };

     QVector<CommentingRule> commentingRules;
     int curCommentingRuleIndex;
};

#endif // SYNTAXHIGHLIGHTER_H
