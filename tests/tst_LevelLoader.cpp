#include <QtTest>
// TODO: implementar testes unitários
class Test : public QObject { Q_OBJECT
private slots:
    void placeholder() { QVERIFY(true); }
};
QTEST_MAIN(Test)
#include "tst_LevelLoader.moc"
