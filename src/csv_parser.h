//
// Created by Benjamin Baron on 08/02/16.
//

#ifndef LOCALL_CSV_PARSER_H
#define LOCALL_CSV_PARSER_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QVector>
#include <QFile>

class CSVParser
{
public:
    CSVParser() {}

    static int parseString(QString &buffer, QStringList &fields, QString delimChars, QString quoteChars, QString escapeChars) {
        QString field;             // String in which to accumulate next field
        bool escaped = false;      // Next char is escaped
        bool quoted = false;       // In quotes
        QChar quoteChar = 0;       // Actual quote character used to open quotes
        bool started = false;      // Non-blank chars in field or quotes started
        bool ended = false;        // Quoted field ended
        QString endChars = "\n\r";
        int cp = 0;                // Pointer to the next character in the buffer
        int cpmax = buffer.size(); // End of string

        while ( true ) {
            QChar c = buffer[cp];
            cp++;

            // If end of line then if escaped or buffered then try to get more...
            if ( cp > cpmax )
            {
                if ( quoted || escaped )
                {
                    return 0;
                }
                break;
            }

            // If escaped, then just append the character
            if ( escaped )
            {
                field.append( c );
                escaped = false;
                continue;
            }

            // Determine if this is a special character - test each class in turn
            // Note that delimiters are not valid as quotes or escape character
            //
            // Somewhat convoluted logic around quote and escape chars is
            // to enforce logic for escape characters that are also quote characters.
            // These escapes can only escape themselves and only in quotes using them
            // as delimiters!

            bool isQuote = false;
            bool isEscape = false;
            bool isDelim = delimChars.contains( c );
            if ( ! isDelim )
            {
                bool isQuoteChar = quoteChars.contains( c );
                isQuote = quoted ? c == quoteChar : isQuoteChar;
                isEscape = escapeChars.contains( c );
                if ( isQuoteChar && isEscape ) isEscape = isQuote;
            }

            // Start or end of quote ...
            if ( isQuote )
            {
                // quote char in quoted field
                if ( quoted )
                {
                    // if is also escape and next character is quote, then
                    // escape the quote..
                    if ( isEscape && buffer[cp] == quoteChar )
                    {
                        field.append( quoteChar );
                        cp++;
                    }
                        // Otherwise end of quoted field
                    else
                    {
                        quoted = false;
                        ended =  true;
                    }
                }
                    // quote char at start of field .. start of quoted fields
                else if ( ! started )
                {
                    field.clear();
                    quoteChar = c;
                    quoted = true;
                    started = true;
                }
                    // Cannot have a quote embedded in a field
                else
                {
                    fields.clear();
                    return 0;
                }
            }
                // If escape char, then next char is escaped...
            else if ( isEscape )
            {
                escaped = true;
            }
                // If within quotes, then append to the string
            else if ( quoted )
            {
                field.append( c );
            }
                // If it is a delimiter, then end of field...
            else if ( isDelim )
            {
                fields.append(field);

                // Clear the field
                field.clear();
                started = false;
                ended = false;
            }
                // End of the string
            else if (endChars.contains(c))
            {
                break;
            }
                // Whitespace is permitted before the start of a field, or
                // after the end..
            else if ( c.isSpace() )
            {
                if ( ! ended ) field.append( c );
            }
                // Other chars permitted if not after quoted field
            else
            {
                if ( ended )
                {
                    fields.clear();
                    return 0;
                }
                field.append( c );
                started = true;
            }
        }
        // If reached the end of the record, then add the last field...
        if ( started )
        {
            fields.append(field);
        }

        return 1;
    }

    static int parseRegEx(QString &buffer, QStringList &fields, QString regEx) {
        QRegExp rx(regEx);

        if(!rx.isValid()) {
            return 0;
        }
        int pos = rx.indexIn(buffer);
        if(pos == -1) {
            return 0;
        }
        QStringList list = rx.capturedTexts();
        for(int i=1;i<list.count();++i){
            fields.append(list[i]);
        }
        return 1;
    }

    static int parseCSV(QString filename, QVector<QMap<QString,QString> > &fields, QString delim, QString quote="\"", QString escape="\\") {
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly)) {
            return 1;
        }

        // The file necessarily has a header
        QStringList headerLine;
        QString line = QString(file.readLine()).split(QRegExp("[\r\n]")).at(0);
        parseString(line,headerLine,delim,quote,escape);
        while (!file.atEnd()) {
            line = QString(file.readLine()).split(QRegExp("[\r\n]")).at(0);
            if(line.isEmpty())
                continue;
            QStringList values;
            parseString(line,values,delim,quote,escape);
            QMap<QString,QString> map = QMap<QString,QString>();
            for (int i = 0; i < values.length(); i++) {
                map.insert(headerLine.at(i), values.at(i)); // maps the header with corresponding value
            }
            fields.append(map);
        }

        file.close();

        return 0;
    }
};



#endif //LOCALL_CSV_PARSER_H
