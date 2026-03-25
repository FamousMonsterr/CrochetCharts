/****************************************************************************\
 Copyright (c) 2010-2014 Stitch Works Software
 Brian C. Milco <bcmilco@gmail.com>

 This file is part of Crochet Charts.

 Crochet Charts is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Crochet Charts is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Crochet Charts. If not, see <http://www.gnu.org/licenses/>.

 \****************************************************************************/
#include "teststitchset.h"

#include <QDir>
#include <QTemporaryDir>

void TestStitchSet::initTestCase()
{
    mSet = new StitchSet();
}

void TestStitchSet::setupStitchSet()
{
    QVERIFY(mSet->stitchCount() == 0);
    QVERIFY(mSet->loadXmlFile(":/crochet.xml"));

    QVERIFY(mSet->stitchCount() > 100);
}

void TestStitchSet::findStitch()
{
    QFETCH(QString, name);
    QFETCH(bool, exists);
    QFETCH(QString, file);
    QFETCH(QString, desc);
    QFETCH(QString, cat);
    QFETCH(QString, ws);

    Stitch* s = mSet->findStitch(name);

    if(!s) {
        if(exists) {
            QFAIL("stitch doesn't exist and it should");
        } else {
            QCOMPARE((bool)s, exists);
            return;
        }
    }

    QVERIFY(s->name() == name);
    QVERIFY(s->file() == file);
    QVERIFY(s->description() == desc);
    QVERIFY(s->category() == cat);
    QVERIFY(s->wrongSide() == ws);
}

void TestStitchSet::findStitch_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<bool>("exists");
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("desc");
    QTest::addColumn<QString>("cat");
    QTest::addColumn<QString>("ws");

    QTest::newRow("sl st") << "sl st" << true << ":/stitches/sl_st.svg" << "slip stitch" << "Default" << "sl st";
    QTest::newRow("ch") << "ch" << true << ":/stitches/ch.svg" << "chain" << "Default" << "ch";
    QTest::newRow("missing") << "does-not-exist" << false << "" << "" << "" << "";
}

void TestStitchSet::saveLoadDataSet()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QString fileName = tempDir.filePath("teststitchset-savefile.set");
    mSet->saveDataFile(fileName);

    StitchSet* testSet = new StitchSet();
    QString destFile = tempDir.filePath("set1.xml");
    QVERIFY(QDir().mkpath(tempDir.filePath("set1")));
    testSet->loadDataFile(fileName, destFile);

    QVERIFY(testSet->stitchCount() == mSet->stitchCount());

    foreach(Stitch* s, mSet->stitches()) {
        Stitch* testSt = testSet->findStitch(s->name());
        QVERIFY(s->name() == testSt->name());
        QVERIFY(s->category() == testSt->category());
        QVERIFY(s->description() == testSt->description());
        QVERIFY(s->file() == testSt->file());
        QVERIFY(s->height() == testSt->height());
        QVERIFY(s->width() == testSt->width());
        QVERIFY(s->isSvg() == testSt->isSvg());
    }

}

void TestStitchSet::cleanupTestCase()
{
    delete mSet;
    mSet = 0;
}
