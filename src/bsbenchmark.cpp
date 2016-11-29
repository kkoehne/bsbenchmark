#include <QString>
#include <QtTest>

class BSBenchmark : public QObject
{
    Q_OBJECT

public:
    BSBenchmark();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void initialBuild_data();
    void initialBuild();
private:
    QTemporaryDir dir;
};

BSBenchmark::BSBenchmark()
{
//    dir.setAutoRemove(false);
//    qDebug() << dir.path();
}

void BSBenchmark::initTestCase()
{
    const int numOfFiles = 1;
    Q_ASSERT(dir.isValid());
    for (int i = 0; i < numOfFiles; ++i) {
        QFile f(dir.path() + "/" + QStringLiteral("file%1.cpp").arg(i));
        f.open(QIODevice::WriteOnly);
        QTextStream str(&f);
        str << "int f" << i << "() { return " << i << "; }\n";
    }

    {
        // qmake
        QFile qbsFile(dir.path() + "/test.pro");
        qbsFile.open(QIODevice::WriteOnly);
        QTextStream qbsFileStream(&qbsFile);
        qbsFileStream << "TEMPLATE = lib\n";
        qbsFileStream << "TARGET = test_qmake\n";
        qbsFileStream << "CONFIG -= qt\n";
        qbsFileStream << "SOURCES += \\\n";
        for (int i = 0; i < numOfFiles; ++i)
            qbsFileStream << "          file" << i << ".cpp\\\n";
    }

    {
        // qbs
        QFile qbsFile(dir.path() + "/test.qbs");
        qbsFile.open(QIODevice::WriteOnly);
        QTextStream qbsFileStream(&qbsFile);
        qbsFileStream << "import qbs 1.0\n"
                         "Library {\n"
                         "      targetName: \"test_qbs\"\n"
                         "      files: [\n";
        for (int i = 0; i < numOfFiles; ++i)
            qbsFileStream << "          \"file" << i << ".cpp\",\n";
        qbsFileStream << "      ]\n"
                         "      Depends { name: \"cpp\" }\n"
                         "}\n";
    }
    {
        // CMake
        QFile qbsFile(dir.path() + "/CMakeLists.txt");
        qbsFile.open(QIODevice::WriteOnly);
        QTextStream qbsFileStream(&qbsFile);
        qbsFileStream << "add_library(test_cmake SHARED\n";
        for (int i = 0; i < numOfFiles; ++i)
            qbsFileStream << "          file" << i << ".cpp\n";
        qbsFileStream << ")\n";
    }

}

void BSBenchmark::cleanupTestCase()
{
}

void BSBenchmark::initialBuild_data()
{
    QTest::addColumn<QString>("binary1");
    QTest::addColumn<QString>("binary1Args");
    QTest::addColumn<QString>("binary2");

    QTest::newRow("qmake/jom") << QString("qmake.exe")
                               << QString("CONFIG+=release")
                               << QString("jom.exe");
    QTest::newRow("cmake/ninja") << QString("cmake.exe")
                                 << QString("-G Ninja -DCMAKE_BUILD_TYPE=Release")
                                 << QString("ninja.exe");
    QTest::newRow("qbs") << QString("qbs.exe")
                         << QString("--command-echo-mode command-line release")
                         << QString();
}

void BSBenchmark::initialBuild()
{
    QFETCH(QString, binary1);
    QFETCH(QString, binary1Args);
    QFETCH(QString, binary2);
    QProcess p1;
    p1.setProgram(binary1);
    p1.setArguments(binary1Args.split(" "));
    p1.setWorkingDirectory(dir.path());
    p1.setProcessChannelMode(QProcess::MergedChannels);
    QProcess p2;
    p2.setProgram(binary2);
    p2.setWorkingDirectory(dir.path());
    p2.setProcessChannelMode(QProcess::MergedChannels);

    // let CMake 'warm up'
//    p1.start();
//    QVERIFY(p1.waitForStarted());
//    QVERIFY(p1.waitForFinished());
//    QVERIFY(p1.exitStatus() == QProcess::NormalExit);


    QBENCHMARK_ONCE {
        p1.start();
        QVERIFY(p1.waitForStarted());
        QVERIFY(p1.waitForFinished());
        QVERIFY(p1.exitStatus() == QProcess::NormalExit);
        if (!p2.program().isEmpty()) {
            p2.start();
            QVERIFY(p2.waitForStarted());
            QVERIFY(p2.waitForFinished());
            QVERIFY(p2.exitStatus() == QProcess::NormalExit);
        }
    }
    if (p1.exitCode() != 0 || p2.exitCode() != 0) {
        qWarning() << binary1 << "returned with" << p1.exitCode();
        qWarning() << "output:\n" << qPrintable(p1.readAll());
        qWarning() << binary2 << "returned with" << p2.exitCode();
        qWarning() << "output:\n" << qPrintable(p2.readAll());
    }
}

QTEST_APPLESS_MAIN(BSBenchmark)

#include "bsenchmark.moc"
