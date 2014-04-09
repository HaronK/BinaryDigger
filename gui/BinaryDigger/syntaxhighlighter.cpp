
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>

#include <QtGui>
#include <QFile>

#include "syntaxhighlighter.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
    , curCommentingRuleIndex(-1)
{
    weightsMap["Light"]    = QFont::Light;
    weightsMap["Normal"]   = QFont::Normal;
    weightsMap["DemiBold"] = QFont::DemiBold;
    weightsMap["Bold"]     = QFont::Bold;
    weightsMap["Black"]    = QFont::Black;

    globalColorMap["color0"]      = Qt::color0;
    globalColorMap["color1"]      = Qt::color1;
    globalColorMap["black"]       = Qt::black;
    globalColorMap["white"]       = Qt::white;
    globalColorMap["darkGray"]    = Qt::darkGray;
    globalColorMap["gray"]        = Qt::gray;
    globalColorMap["lightGray"]   = Qt::lightGray;
    globalColorMap["red"]         = Qt::red;
    globalColorMap["green"]       = Qt::green;
    globalColorMap["blue"]        = Qt::blue;
    globalColorMap["cyan"]        = Qt::cyan;
    globalColorMap["magenta"]     = Qt::magenta;
    globalColorMap["yellow"]      = Qt::yellow;
    globalColorMap["darkRed"]     = Qt::darkRed;
    globalColorMap["darkGreen"]   = Qt::darkGreen;
    globalColorMap["darkBlue"]    = Qt::darkBlue;
    globalColorMap["darkCyan"]    = Qt::darkCyan;
    globalColorMap["darkMagenta"] = Qt::darkMagenta;
    globalColorMap["darkYellow"]  = Qt::darkYellow;
    globalColorMap["transparent"] = Qt::transparent;
}

void SyntaxHighlighter::initFormatReference(const QJsonObject& templ, QTextCharFormat& format)
{
    if (templ.contains("format"))
    {
        QJsonValue value = templ.value("format");
        QString name = value.toString();

        FormatsMap::const_iterator iter = formatsMap.find(name);
        if (iter != formatsMap.end())
        {
//            format = iter->second; // FIXME: doesn't work. Why?

            format.setFontWeight(iter->second.fontWeight());
            format.setForeground(iter->second.foreground());
            format.setFontItalic(iter->second.fontItalic());
        }
        else
        {
            // TODO: print some warning if needed
        }
    }
}

bool SyntaxHighlighter::getWeight(const QJsonObject& templ, QFont::Weight& res) const
{
    if (templ.contains("weight"))
    {
        QJsonValue value = templ.value("weight");
        QString weight = value.toString();

        WeightsMap::const_iterator iter = weightsMap.find(weight);
        if (iter != weightsMap.end())
        {
            res = iter->second;
            return true;
        }
    }
    return false;
}

bool SyntaxHighlighter::getForeground(const QJsonObject& templ, Qt::GlobalColor& res) const
{
    if (templ.contains("fg"))
    {
        QJsonValue value = templ.value("fg");
        QString foreground = value.toString();

        GlobalColorMap::const_iterator iter = globalColorMap.find(foreground);
        if (iter != globalColorMap.end())
        {
            res = iter->second;
            return true;
        }
    }
    return false;
}

bool SyntaxHighlighter::getItalic(const QJsonObject& templ, bool& res) const
{
    if (templ.contains("italic"))
    {
        QJsonValue value = templ.value("italic");
        res = value.toBool();
        return true;
    }
    return false;
}

void SyntaxHighlighter::setFormatObject(const QJsonObject& templ, QTextCharFormat& format)
{
    initFormatReference(templ, format);

    // TODO: set defaults properly
    QFont::Weight weight = QFont::Normal;
    if (getWeight(templ, weight))
        format.setFontWeight(weight);

    Qt::GlobalColor fg = Qt::black;
    if (getForeground(templ, fg))
        format.setForeground(fg);

    bool italic = false;
    if (getItalic(templ, italic))
        format.setFontItalic(italic);
}

void SyntaxHighlighter::initFormatTemplate(const QString& name, const QJsonObject& templ)
{
    QTextCharFormat format;

    setFormatObject(templ, format);

    formatsMap[name] = format;
}

void SyntaxHighlighter::initRegexpsTemplate(const QString& /*name*/, const QJsonObject& templ)
{
    QString regexp = templ["regexp"].toString();
    if (regexp.isEmpty())
        return; // TODO: show some syntax error message

    HighlightingRule rule;

    setFormatObject(templ, rule.format);

    rule.pattern = QRegExp(regexp);

    highlightingRules.append(rule);
}

void SyntaxHighlighter::initKeywordsTemplate(const QString& /*name*/, const QJsonObject& templ)
{
    HighlightingRule rule;

    setFormatObject(templ, rule.format);

    QJsonArray wordsList = templ["list"].toArray();

    for (int j = 0; j < wordsList.size(); ++j)
    {
        QString word = wordsList[j].toString();
        if (word.isEmpty())
            continue; // TODO: show some syntax error message

        QString pattern("\\b");
        pattern.append(word);
        pattern.append("\\b");

        rule.pattern = QRegExp(pattern);

        highlightingRules.append(rule);
    }
}

void SyntaxHighlighter::initCommentsTemplate(const QString& /*name*/, const QJsonObject& templ)
{
    QString beginRegexp = templ["beginRegexp"].toString();
    if (beginRegexp.isEmpty())
        return; // TODO: show some syntax error message

    bool multilined = templ["multilined"].toBool(false);

    QString endRegexp = templ["endRegexp"].toString();
    if (!multilined)
    {
        if (!endRegexp.isEmpty())
            endRegexp += "|";
        endRegexp += "$";
    }

    CommentingRule rule;

    setFormatObject(templ, rule.format);

    rule.beginRegexp = QRegExp(beginRegexp);
    rule.endRegexp = QRegExp(endRegexp);

    commentingRules.append(rule);
}

void SyntaxHighlighter::initSyntax(const QString& syntaxFilePath)
{
    cleanSyntax();

    QFile syntaxFile(syntaxFilePath);
    if (!syntaxFile.open(QIODevice::ReadOnly))
    {
        qWarning("Couldn't open syntax file.");
        return;
    }

    QByteArray saveData = syntaxFile.readAll();
    QJsonDocument syntaxDoc(QJsonDocument::fromJson(saveData));
    QJsonObject syntax = syntaxDoc.object();

    QJsonObject formatsTemplates = syntax["formats"].toObject();
    for (QJsonObject::iterator iter = formatsTemplates.begin(); iter != formatsTemplates.end(); ++iter)
    {
        initFormatTemplate(iter.key(), iter.value().toObject());
    }

    QJsonObject regexpsTemplates = syntax["regexps"].toObject();
    for (QJsonObject::iterator iter = regexpsTemplates.begin(); iter != regexpsTemplates.end(); ++iter)
    {
        initRegexpsTemplate(iter.key(), iter.value().toObject());
    }

    QJsonObject keywordsTemplates = syntax["keywords"].toObject();
    for (QJsonObject::iterator iter = keywordsTemplates.begin(); iter != keywordsTemplates.end(); ++iter)
    {
        initKeywordsTemplate(iter.key(), iter.value().toObject());
    }

    QJsonObject commentsTemplates = syntax["comments"].toObject();
    for (QJsonObject::iterator iter = commentsTemplates.begin(); iter != commentsTemplates.end(); ++iter)
    {
        initCommentsTemplate(iter.key(), iter.value().toObject());
    }
}

void SyntaxHighlighter::cleanSyntax()
{
    highlightingRules.clear();
    commentingRules.clear();
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules)
    {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);

        while (index >= 0)
        {
            int length = expression.matchedLength();

            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }

    highlightComments(text);
}

int SyntaxHighlighter::findClosestComment(const QString& text, int startIndex, int& ruleIndex)
{
    int resStartIndex = text.size();
    for (int i = 0; i < commentingRules.size(); ++i)
    {
        int curStartIndex = commentingRules[i].beginRegexp.indexIn(text, startIndex);
        if (curStartIndex != -1 && curStartIndex < resStartIndex)
        {
            resStartIndex = curStartIndex;
            ruleIndex = i;
        }
    }
    return resStartIndex < text.size() ? resStartIndex : -1;
}

void SyntaxHighlighter::highlightComments(const QString &text)
{
    setCurrentBlockState(0);

    int startIndex = 0;

    if (previousBlockState() != 1)
        startIndex = findClosestComment(text, startIndex, curCommentingRuleIndex);

    while (startIndex >= 0)
    {
        int endIndex = commentingRules[curCommentingRuleIndex].endRegexp.indexIn(text, startIndex + 1);
        int commentLength;

        if (endIndex == -1)
        {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        }
        else
        {
            commentLength = endIndex - startIndex + commentingRules[curCommentingRuleIndex].endRegexp.matchedLength();
        }

        setFormat(startIndex, commentLength, commentingRules[curCommentingRuleIndex].format);

        startIndex = findClosestComment(text, startIndex + commentLength, curCommentingRuleIndex);
    }
}
