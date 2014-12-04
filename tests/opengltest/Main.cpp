///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2014) Alexander Stukowski
//
//  This file is part of OVITO (Open Visualization Tool).
//
//  OVITO is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  OVITO is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////

#include <core/Core.h>
#include "TestWindow.h"
#include "Window1.h"
#include "Window2.h"
#include "Window3.h"
#include "Window4.h"
#include "Window5.h"
#include "Window6.h"
#include "Window7.h"
#include "Window8.h"
#include "Window9.h"
#include "Window10.h"
#include "Window11.h"

int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	QWidget* mainWindow = new QWidget();
	mainWindow->setWindowTitle("OpenGL Compatibility Test");
	mainWindow->setAttribute(Qt::WA_DeleteOnClose);
	mainWindow->setMinimumSize(800, 600);

	static QTextEdit* logWindow = new QTextEdit(mainWindow);
	logWindow->setReadOnly(true);

	qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &msg) {
		logWindow->append(msg);
		if(type == QtFatalMsg)
			abort();
	});

	QVBoxLayout* mainlayout = new QVBoxLayout(mainWindow);

	QGridLayout* layout = new QGridLayout();
	mainlayout->addLayout(layout, 1);
	mainlayout->addWidget(logWindow);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, mainWindow);
	QObject::connect(buttonBox, &QDialogButtonBox::rejected, [mainWindow]() { mainWindow->close(); });
	QObject::connect(buttonBox->addButton(QString("Copy log to clipboard"), QDialogButtonBox::ActionRole), &QPushButton::clicked,
			[]() { QApplication::clipboard()->setText(logWindow->toPlainText()); });
	QObject::connect(buttonBox->addButton(QString("Save log to file..."), QDialogButtonBox::ActionRole), &QPushButton::clicked, [mainWindow]() {
		QString fileName = QFileDialog::getSaveFileName(mainWindow, QString("Save Log"), QString(), QString("Text files (*.txt);;All files (*)"));
		if(fileName.isEmpty()) return;
		QFile file(fileName);
		file.open(QIODevice::WriteOnly | QIODevice::Text);
		file.write(logWindow->toPlainText().toLocal8Bit());
	});
	QObject::connect(buttonBox->addButton(QString("Copy screenshot to clipboard"), QDialogButtonBox::ActionRole), &QPushButton::clicked,
			[mainWindow]() {
		QApplication::clipboard()->setPixmap(
				QGuiApplication::allWindows().front()->screen()->grabWindow(mainWindow->winId()).copy(mainWindow->frameGeometry()));
	});
	mainlayout->addWidget(buttonBox);

	layout->setSpacing(10);
	QList<TestWindow*> windows;

	windows.push_back(new Window1());
	windows.push_back(new Window2());
	windows.push_back(new Window3());
	windows.push_back(new Window4());
	windows.push_back(new Window5());
	windows.push_back(new Window6());
	windows.push_back(new Window7());
	windows.push_back(new Window8());
	windows.push_back(new Window9());
	windows.push_back(new Window10());
	windows.push_back(new Window11());

	for(int i = 0; i < windows.size(); i++) {
		QWidget* widget = QWidget::createWindowContainer(windows[i], mainWindow);
		widget->setMinimumSize(100, 100);
		layout->addWidget(widget, i/4, i%4);
	}

	mainWindow->show();

	return app.exec();
}
