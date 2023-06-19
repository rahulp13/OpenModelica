#include "VSSDialog.h"
#include <string>
#include <fstream>
#include <iostream>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QDateTime>
#include <QTextStream>
#include <QMessageBox>
#include <QGuiApplication>

QString osName() {
  #if defined(Q_OS_WIN)
    return QLatin1String("windows");
  #elif defined(Q_OS_LINUX)
    return QLatin1String("linux");
  #else
    return QLatin1String("unknown");
  #endif
}

QString VSSDialog::pythonScriptName() {
  std::string schedApp;

  try {
    std::ifstream schedAppFile;
    schedAppFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    schedAppFile.open((mOMSchedPath + QDir::separator() + QString("app-launch.txt")).toStdString());
    getline(schedAppFile, schedApp);
    schedAppFile.close();
  } catch (const std::ifstream::failure& e) {
    QMessageBox::critical(this, QGuiApplication::applicationDisplayName(), "Error! Cannot find launch file.");
    return "error";
  }

  return QString::fromStdString(schedApp);
}

QString VSSDialog::pythonExecPath(QString system) {
  QProcess sysProcc;

  // Call command depending on platform
  if (system == "linux") {
    sysProcc.start("which", {"python3"});
  } else if (system == "windows") {
    sysProcc.start("where", {"python3"});
  } else {
    return "";
  }

  sysProcc.waitForFinished();
  // Check that if it was a sucessful call
  int retCode = sysProcc.exitCode();
  // Define python path
  QString pythonPath;
  if (retCode == 0) {
    // Get STDOUT
    QString sysCallSTDOUT(sysProcc.readAllStandardOutput());
    // Sanitize output
    QStringList paths = sysCallSTDOUT.split("\n");
    QString firstPath = paths.at(0);
    pythonPath = firstPath;
  }
  else {
    pythonPath = "?";
  }

  return pythonPath.trimmed();
}

VSSDialog::VSSDialog(QWidget *parent, LibraryTreeItem *pLibraryTreeItem, LibraryTreeModel *pLibraryTreeModel, QString workDir) : QDialog(parent) {
  // Dialog settings
  appName = "OMScheduler";
  mpLibraryTreeItem = pLibraryTreeItem;
  mpLibraryTreeModel = pLibraryTreeModel;
  workingDirectory = workDir;
  eventFlag = QEventLoop::ExcludeUserInputEvents;

  setMinimumWidth(400);
  setWindowTitle(appName);

  QString system = osName();
  QString homePath;
  if (system == "linux") {
    homePath = getenv("HOME");
  } else if (system == "windows") {
    homePath = getenv("USERPROFILE");
  } else {
    homePath = "";
  }

  // OMScheduler python backend path
  mOMSchedPath = QDir::cleanPath(homePath);
  // Python executable path
  mPythonBinPath = QDir::cleanPath(pythonExecPath(system));
  // Initialize paths
  mpOMSchedPathLabel = new QLabel(appName + " Python Backend Folder:");
  mpOMSchedPathValue = new QLabel(mOMSchedPath);
  mpOMSchedPathValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  mpOMSchedPathBrowseButton = new QPushButton("Browse");
  mpOMSchedPathBrowseButton->setAutoDefault(true);
  mpOMSchedPathBrowseButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
  connect(mpOMSchedPathBrowseButton, SIGNAL(clicked()), this, SLOT(launchOMSchedBackendChooseFolderDialog()));

  mpPythonBinLabel = new QLabel("Python Executable:");
  mpPythonBinValue = new QLabel(mPythonBinPath);
  mpPythonBinValue->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  mpPythonBinBrowseButton = new QPushButton("Browse");
  mpPythonBinBrowseButton->setAutoDefault(true);
  mpPythonBinBrowseButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
  connect(mpPythonBinBrowseButton, SIGNAL(clicked()), this, SLOT(launchPythonBinChooseFolderDialog()));

  // Division between paths and buttons
  mpHorizontalLineOne= new QFrame;
  mpHorizontalLineOne->setFrameShape(QFrame::HLine);
  mpHorizontalLineOne->setFrameShadow(QFrame::Sunken);

  // Help (not shown for now)
  mpHelpButton = new QPushButton("Help");
  mpHelpButton->setAutoDefault(true);
  mpHelpButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
  connect(mpHelpButton, SIGNAL(clicked()), this, SLOT(launchHelpMessageDialog()));

  // Load OMScheduler
  mpLoadButton = new QPushButton("Load");
  mpLoadButton->setAutoDefault(true);
  mpLoadButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
  connect(mpLoadButton, SIGNAL(clicked()), this, SLOT(launchOMSched()));

  // Layout
  QVBoxLayout *mainLayout = new QVBoxLayout;
  // OMSched folder
  mainLayout->addWidget(mpOMSchedPathLabel, 0, Qt::AlignLeft);
  QHBoxLayout *pOMSchedValueLayout = new QHBoxLayout;
  pOMSchedValueLayout->addWidget(mpOMSchedPathValue);
  pOMSchedValueLayout->addWidget(mpOMSchedPathBrowseButton);
  mainLayout->addLayout(pOMSchedValueLayout);
  // Python bin
  mainLayout->addWidget(mpPythonBinLabel, 0, Qt::AlignLeft);
  QHBoxLayout *pPythonBinValueLayout = new QHBoxLayout;
  pPythonBinValueLayout->addWidget(mpPythonBinValue);
  pPythonBinValueLayout->addWidget(mpPythonBinBrowseButton);
  mainLayout->addLayout(pPythonBinValueLayout);
  // Division
  mainLayout->addWidget(mpHorizontalLineOne);
  // mainLayout->addWidget(mpLoadButton , 0, Qt::AlignCenter);

  // Buttons
  QHBoxLayout *pButtonLayout = new QHBoxLayout;
  pButtonLayout->addWidget(mpHelpButton, 0, Qt::AlignCenter);
  pButtonLayout->addWidget(mpLoadButton , 0, Qt::AlignCenter);
  mainLayout->addLayout(pButtonLayout);

  // Layout settings
  // this->setWindowModality(Qt::WindowModal);
  // this->setModal(true);
  mainLayout->setAlignment(Qt::AlignCenter);
  setLayout(mainLayout);
}

void VSSDialog::launchOMSchedBackendChooseFolderDialog() {
  // Launch dialog
  QString path = QFileDialog::getExistingDirectory(
    this, "Choose Destination Folder",
    mOMSchedPath, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
  );

  if(!path.isEmpty() && !path.isNull()) {
    mOMSchedPath = path;
    mpOMSchedPathValue->setText(mOMSchedPath);
  }
}

void VSSDialog::launchPythonBinChooseFolderDialog() {
  // Launch dialog
  QString path = QFileDialog::getOpenFileName(
    this, "Choose Python Interpreter",
    mPythonBinPath, "Python interpreter(*)"
  );

  if(!path.isEmpty() && !path.isNull()) {
    mPythonBinPath = path;
    mpPythonBinValue->setText(mPythonBinPath);
  }
}

void VSSDialog::launchHelpMessageDialog() {
  QMessageBox::information(
    this, appName,
    "'OMScheduler' facilitates the sequential modular simulations "
    "and the simulation of variable-structure systems. <br/>"
    "The project is compliant with the Modelica standards.<br/><br/>"
    "To install it on your system and for further information, visit <a href="
    "https://www.github.com/rahulp13/OMSched.git"
    ">OMScheduler GitHub</a>.<br/>"
  );
}

QString VSSDialog::dirPathForFilePath(QString scriptPath) {
  QFileInfo scriptFileInfo = QFileInfo(scriptPath);
  QDir      scriptDir          = scriptFileInfo.canonicalPath();
  QString scriptDirPath        = scriptDir.canonicalPath();

  return scriptDirPath;
}

QString VSSDialog::progressDialogText() {
  QDateTime currentTime = QDateTime::currentDateTime();
  QString date = currentTime.toString("dd/MM/yyyy");
  QString h_m_s = currentTime.toString("H:m:s");
  QString scriptRunStartString = "(started on " + date + " at " + h_m_s + ")";
  QString progressDialogText = "Launching OMScheduler Python Engine ... " + scriptRunStartString;

  return progressDialogText;
}

void VSSDialog::getResultFileData(QTimer* timer) {
  // Lock the shared memory segment for reading
  if (!sharedMemory.lock()) {
    qDebug() << "Failed to lock shared memory segment:" << sharedMemory.errorString();
    return;
  }

  // Read the result data from the shared memory with engine
  const QByteArray readByteArray(static_cast<const char*>(sharedMemory.constData()), sharedMemory.size());
  QString data = QString::fromUtf8(readByteArray);

  sharedMemory.unlock();

  if (data.size() > 5) {
    timer->stop();

    QStringList resultList = data.split("::");
    QString resultFileInfo = resultList.at(0);
    int resultFileCount = resultList.at(1).toInt();

    QFile resultInfoFile(resultFileInfo);
    if (resultInfoFile.open(QIODevice::ReadOnly | QIODevice::Text)) {

      int progressValue = 0;
      QProgressBar* pProgressBar = MainWindow::instance()->getProgressBar();
      pProgressBar->setRange(0, resultFileCount);
      MainWindow::instance()->showProgressBar();

      eventFlag = QEventLoop::AllEvents;

      QTextStream resultFileList(&resultInfoFile);
      QString resultFileName;
      while (!resultFileList.atEnd()) {
        resultFileName = resultFileList.readLine();

        pProgressBar->setValue(++progressValue);
        MainWindow::instance()->openResultFile(resultFileName);
      }

      resultInfoFile.close();
      MainWindow::instance()->hideProgressBar();
    }
  }

  qDebug() << data;
}

void VSSDialog::launchOMSched() {
  QString scriptFileName = pythonScriptName();
  if (scriptFileName == "error") {
    return;
  }

  // Initialize dialog showing progress
  QProgressDialog* dialog = new QProgressDialog(progressDialogText(), NULL, 0, 0, this);
  dialog->setWindowTitle(appName);
  dialog->setAttribute(Qt::WA_DeleteOnClose);

  QPushButton cancelButton("Cancel");
  connect(&cancelButton, &QPushButton::clicked, dialog, &QProgressDialog::cancel);

  QTimer* timer = new QTimer(this);
  timer->setSingleShot(true);
  connect(timer, SIGNAL(timeout()), dialog, SLOT(accept()));

  this->hide();
  dialog->setWindowModality(Qt::WindowModal);
  dialog->setModal(true);
  dialog->show();
  timer->start(3500);

  QString scriptPath = QDir::cleanPath(mOMSchedPath + QDir::separator() + scriptFileName);
  QString scriptDirPath = dirPathForFilePath(scriptPath);
  QString packagePath = mpLibraryTreeModel->getTopLevelLibraryTreeItem(mpLibraryTreeItem)->getFileName();
  QStringList args = {scriptPath, "-m", mpLibraryTreeItem->getNameStructure(), '"' + packagePath + '"'};

  QStringList classes = MainWindow::instance()->getOMCProxy()->getClassNames();
  LibraryTreeItem *pModelLibraryTreeItem;
  foreach (QString modelName, classes) {
    pModelLibraryTreeItem = mpLibraryTreeModel->findLibraryTreeItemOneLevel(modelName);
    if (pModelLibraryTreeItem && pModelLibraryTreeItem->getFileName() != packagePath) {
      args.append('"' + pModelLibraryTreeItem->getFileName() + '"');
    }
  }
  args.append('"' + workingDirectory + '"');

  QProcess pythonSchedProcess;
  pythonSchedProcess.setWorkingDirectory(scriptDirPath);
  pythonSchedProcess.setProcessChannelMode(QProcess::MergedChannels);
  QString command = mPythonBinPath;
  pythonSchedProcess.start(command, args);

  // connect(&pythonSchedProcess, SIGNAL(finished(int)), dialog, SLOT(close()));
  connect(dialog, SIGNAL(canceled()), &pythonSchedProcess, SLOT(kill()));
  connect(qApp, SIGNAL(aboutToQuit()), &pythonSchedProcess, SLOT(kill()));

  sharedMemory.setKey("resultFile");
  sharedMemory.create(4096);
  QTimer* memTimer = new QTimer(this);
  memTimer->setInterval(5000);
  connect(memTimer, &QTimer::timeout, [this, memTimer](){
    this->getResultFileData(memTimer);
  });
  memTimer->start();

  // Wait for the process to finish
  do {
    qApp->processEvents(eventFlag);
  } while (!pythonSchedProcess.waitForFinished(100)); //  1/10 second

  // See if the process ended correctly
  QProcess::ExitStatus exitStatus = pythonSchedProcess.exitStatus();
  int exitCode = pythonSchedProcess.exitCode();

  // Prepare python call log
  QString python_call_output(pythonSchedProcess.readAllStandardOutput());
  QString pythonLog = QString("full command: %1  %2\n\n%3\n\n%4").arg(command, args.join(" "), pythonSchedProcess.errorString(), python_call_output);

  // Write log to file
  QString logFileName = "python_log.txt";
  QString logPath = QDir::cleanPath(scriptDirPath + QDir::separator() + logFileName);
  QFile logFile(logPath);
  if (logFile.open(QIODevice::ReadWrite)) {
    QTextStream out(&logFile);
    out << pythonLog;
    logFile.close();
  }

  bool processEndedCorrectly = (exitStatus == QProcess::NormalExit) && (exitCode == 0);
  if (!processEndedCorrectly) {
    if (dialog) {
      dialog->close();
    }

    QMessageBox::critical(this, appName + " -- Error", python_call_output);
  }

  timer->stop();
  memTimer->stop();
  sharedMemory.detach();
  this->show();
}
