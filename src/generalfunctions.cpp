/*
 *
 * This file is part of the Notepadqq text editor.
 *
 * Copyright(c) Notepadqq team.
 * http://notepadqq.sourceforge.net/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "generalfunctions.h"
#include <QStringList>
#include <QProcess>
#include <QTextDecoder>


generalFunctions::generalFunctions()
{

}

QString generalFunctions::getFileMime(QString file)
{
    // Yeah, we suck at this
    return getOutputFromFileMimeCmd(file, "--mime-type");
}

QString generalFunctions::getFileEncoding(QString file)
{
    QString enc = getOutputFromFileMimeCmd(file, "--mime-encoding");
    if(enc != "")
    {
        for(int i=0; i<QTextCodec::availableCodecs().count(); i++)
        {
            if(QTextCodec::availableCodecs().at(i).toLower() == enc.toLower())
            {
                return QTextCodec::availableCodecs().at(i);
            }
        }
    }

    // Codec not available
    enc = "UTF-8";
    return enc;
}

QString generalFunctions::getOutputFromFileMimeCmd(QString file, QString mimeArg)
{
    try {
      QProcess *process = new QProcess();
      QStringList *args = new QStringList();
      args->append("--brief");
      args->append(mimeArg);
      args->append(file);
      process->start("file", *args);
      if(process->waitForStarted(2000))
      {
          process->closeWriteChannel();
          process->waitForFinished(2000);
          QByteArray qba = process->readAll();
          QTextCodec *codec = QTextCodec::codecForLocale();
          QTextDecoder *decoder = codec->makeDecoder();
          QString result = decoder->toUnicode(qba);

          delete args;
          delete decoder;
          delete process;

          result = result.trimmed();
          return result;
      } else
      {
          return "";
      }

    }
    catch (...) {
       return "";
    }
}

QString generalFunctions::readDConfKey(QString schema, QString key)
{
    try {
      QProcess *process = new QProcess();
      QStringList *args = new QStringList();
      args->append("get");
      args->append(schema);
      args->append(key);
      process->start("gsettings", *args);
      if(process->waitForStarted(2000))
      {
          process->closeWriteChannel();
          process->waitForFinished(2000);
          QByteArray qba = process->readAll();
          QTextCodec *codec = QTextCodec::codecForLocale();
          QTextDecoder *decoder = codec->makeDecoder();
          QString result = decoder->toUnicode(qba);

          delete args;
          delete decoder;
          delete process;

          result = result.trimmed().mid(1, result.length()-3);
          return result;
      } else {
          return "";
      }
    }
    catch (...) {
       return "";
    }
}
