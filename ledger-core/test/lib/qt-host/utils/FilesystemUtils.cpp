/*
 *
 * FilesystemUtils.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 20/09/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ledger
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "FilesystemUtils.hpp"
#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include <iostream>

void ledger::qt::FilesystemUtils::clearFs(const std::string& path) {
    QDir root(QString::fromStdString(path));
    for (const auto& file : root.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot)) {
        if (file.isDir() && file.absoluteFilePath().compare(root.absolutePath()) != 0) {
            clearFs(file.absoluteFilePath().toStdString());
        } else if (!file.isExecutable()) {
            QFile::remove(file.absoluteFilePath());
        }
    }
}
