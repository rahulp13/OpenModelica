#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QProcess>
#include <QString>
#include <QSharedMemory>
#include <QProgressDialog>
#include <QTimer>

#include "MainWindow.h"
#include "Modeling/LibraryTreeWidget.h"

class VSSDialog : public QDialog {
  Q_OBJECT

  public:
    VSSDialog(QWidget *parent = 0, LibraryTreeItem *mpLibraryTreeItem = 0, LibraryTreeModel* mpLibraryTreeModel = 0, QString workingDirectory = 0);

  private:
    // Data
    QString mOMSchedPath;
    QString mPythonBinPath;
    QString appName;
    QString workingDirectory;
    QSharedMemory sharedMemory;
    QEventLoop::ProcessEventsFlags eventFlag;

    // GUI
    QLabel      *mpOMSchedPathLabel;
    QLabel      *mpOMSchedPathValue;
    QPushButton *mpOMSchedPathBrowseButton;
    QLabel      *mpPythonBinLabel;
    QLabel      *mpPythonBinValue;
    QPushButton *mpPythonBinBrowseButton;
    QFrame      *mpHorizontalLineOne;
    QPushButton *mpHelpButton; // Not shown for now
    QPushButton *mpLoadButton;
    LibraryTreeItem *mpLibraryTreeItem;
    LibraryTreeModel *mpLibraryTreeModel;

    // Auxs
    QString pythonScriptName();
    QString createTimestampDir(QString);
    QString progressDialogText();
    QString dirPathForFilePath(QString);
    QString omSchedBackendPath();
    QString pythonExecPath(QString);
    bool runProcessAndShowProgress(QString, QString, QStringList, QString);

  public slots:
    void launchOMSchedBackendChooseFolderDialog();
    void launchPythonBinChooseFolderDialog();
    void launchHelpMessageDialog();
    void getResultFileData(QTimer *);
    void launchOMSched();
};
