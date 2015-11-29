#ifndef SMTSOLVER_H
#define SMTSOLVER_H

#include <QDebug>
#include <QFile>
#include <QTextStream>

#include <errno.h>
#include <zlib.h>

#include "minisat/utils/System.h"
#include "minisat/utils/ParseUtils.h"
#include "minisat/utils/Options.h"
#include "minisat/core/Dimacs.h"
#include "minisat/simp/SimpSolver.h"

class SMTSolver
{
public:
    SMTSolver();
    QString convertToBinary(int integer)
    {
        QString res = QString::number(integer, 2);
        int lengthOfBinary = res.length();
        if (lengthOfBinary < BIT_LENGTH)
        {
            for (int i = 0; i < (BIT_LENGTH - lengthOfBinary); i++ )
            {
                res.push_front("0");
            }

        }
        return res;
    }

    //generate sum rule to file. rule of x xor y = z
    QString generateSumRule(QString __x, QString __y, QString __z)
    {

        qDebug() << __x << "+" << __y << "=" << __z;

    //    QString carry = QString("c_").append("%1+%2").arg(__x).arg(__y);
        QString carry = QString("carry").append("%1").arg(__z);

        QString result = "";

        QString sum0 = QString("!%1_0 || %2_0 || %3_0 \n!%1_0 || !%2_0 || !%3_0 \n!%2_0 || %3_0 || %1_0 \n%2_0 || !%3_0 || %1_0\n").arg(__z).arg(__x).arg(__y);
        result.append(sum0);
        QString carry0 = QString("!%1_0 || %2_0 \n!%1_0 || %3_0 \n!%2_0 || !%3_0 || %1_0\n").arg(carry).arg(__x).arg(__y);
        result.append(carry0);
        for (int index = 1; index < BIT_LENGTH; index++)
        {
            QString sumIndex = QString("!%1_%5 || %2_%5 || !%3_%5 || !%4_%6 \n!%1_%5 || !%2_%5 || %3_%5 || !%4_%6 \n!%1_%5 || !%2_%5 || !%3_%5 || %4_%6 \n!%1_%5 || %2_%5 || %3_%5 || %4_%6 \n!%2_%5 || %3_%5 || %4_%6 || %1_%5 \n%2_%5 || !%3_%5 || %4_%6 || %1_%5 \n%2_%5 || %3_%5 || !%4_%6 || %1_%5 \n!%2_%5 || !%3_%5 || !%4_%6 || %1_%5\n")
                    .arg(__z).arg(__x).arg(__y).arg(carry).arg(index).arg(index-1);
            result.append(sumIndex);

            QString carryIndex = QString("!%1_%4 || %2_%4 || %3_%4 \n!%1_%4 || %2_%4 || %1_%5 \n!%1_%4 || %3_%4 || %1_%5 \n!%2_%4 || !%3_%4 || %1_%4 \n!%2_%4 || !%1_%5 || %1_%4 \n!%3_%4 || !%1_%5 || %1_%4\n").arg(carry).arg(__x).arg(__y).arg(index).arg(index-1);
            result.append(carryIndex);
        }

        QString sumAtBitEnd = QString("!%1_%3 || %2_%4 \n!%2_%4 || %1_%3\n").arg(__z).arg(carry).arg(BIT_LENGTH).arg(BIT_LENGTH-1);
        result.append(sumAtBitEnd);

        bool isXInt = false, isYInt = false, isZInt = false;
        int x_value = __x.toInt(&isXInt);
        int y_value = __y.toInt(&isYInt);
        int z_value = __z.toInt(&isZInt);

    //    qDebug() << isXInt << " " << isYInt << " " << isZInt;

        QString binary;
        if (isXInt)
        {
            binary = convertToBinary(x_value);
            for (int i = BIT_LENGTH - 1; i >= 0; i--)
            {
                QString xIndex = QString(binary.at(i)).toInt() == 0 ? "!" : "";
                xIndex.append(QString("%1_%2\n").arg(__x).arg(BIT_LENGTH - i - 1));
                result.append(xIndex);
            }
        }

        if (isYInt)
        {
            binary = convertToBinary(y_value);
            for (int i = BIT_LENGTH - 1; i >= 0; i--)
            {
                QString yIndex = QString(binary.at(i)).toInt() == 0 ? "!" : "";
                yIndex.append(QString("%1_%2\n").arg(__y).arg(BIT_LENGTH - i - 1));
                result.append(yIndex);
            }
        }

        if (isZInt)
        {
            binary = convertToBinary(z_value);
            for (int i = BIT_LENGTH - 1; i >= 0; i--)
            {
                QString zIndex = QString(binary.at(i)).toInt() == 0 ? "!" : "";
                zIndex.append(QString("%1_%2\n").arg(__z).arg(BIT_LENGTH - i - 1));
                result.append(zIndex);
            }
        }

        return result;
    }



    //generate equal rule of two number
    QString generateEqualRule(QString __x, QString __y)
    {
        qDebug() << __x << " = " << __y;

        QString result = "";
        for(int i=0; i<BIT_LENGTH; i++)
        {
            QString rule = QString("!%1_%3 || %2_%3 \n !%2_%3 || %1_%3 \n").arg(__x).arg(__y).arg(i);
            result.append(rule);
        }


        bool isXInt = false, isYInt = false;
        int x_value = __x.toInt(&isXInt);
        int y_value = __y.toInt(&isYInt);

        QString binary;
        if (isXInt)
        {
            binary = convertToBinary(x_value);
            for (int i = BIT_LENGTH - 1; i >= 0; i--)
            {
                QString xIndex = QString(binary.at(i)).toInt() == 0 ? "!" : "";
                xIndex.append(QString("%1_%2\n").arg(__x).arg(BIT_LENGTH - i - 1));
                result.append(xIndex);
            }
        }

        if (isYInt)
        {
            binary = convertToBinary(y_value);
            for (int i = BIT_LENGTH - 1; i >= 0; i--)
            {
                QString yIndex = QString(binary.at(i)).toInt() == 0 ? "!" : "";
                yIndex.append(QString("%1_%2\n").arg(__y).arg(BIT_LENGTH - i - 1));
                result.append(yIndex);
            }
        }

        return result;
    }

    //read sum rule file to generate CNF file
    void generateCnfFlie()
    {

        QFile file(ruleFileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug() << "cannot open file";
            return;
        }
        QTextStream stream(&file);

        QFile clauseFile(cnfFileName);
        if (!clauseFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qDebug() << "cannot open file";
            return;
        }
        QTextStream cnfStream(&clauseFile);
        QString resultCnfClause = "";
        int numberOfClause = 0;
        while (!stream.atEnd()) {
            QString line = stream.readLine();
            QStringList elements = line.split(" || ");
    //        qDebug() << "elements:" << elements;
            QString cnfLine = "";
            foreach (QString ele, elements)
            {
                bool isNOT = false;
                QString varLogical = ele.trimmed();
                if (ele.contains("!"))
                {
                    varLogical = varLogical.split("!").at(1);
                    isNOT = true;
                }
                if (!mapBitVariables.values().contains(varLogical))
                {
                    mapBitVariables.insert(globalIndex++,varLogical);
                }
                cnfLine.append(QString("%1 ").arg(isNOT ? - mapBitVariables.key(varLogical) : mapBitVariables.key(varLogical)));
            }
            cnfLine.append("0");
            numberOfClause++;
            resultCnfClause.append(cnfLine).append("\n");
        }
        resultCnfClause.push_front(QString("p cnf %1 %2\n").arg(mapBitVariables.keys().count()).arg(numberOfClause));
        cnfStream << resultCnfClause;
        file.close();
        clauseFile.close();
    }

    void runSATSolver(std::string resultFileName, Minisat::SimpSolver& S);





    void multiNotContainStar(QString input, int &coef, QString &varText)
    {
        QString coefText="";
        input = input.trimmed();
        for(int i = 0; i < input.length(); i++)
        {
            QChar charAtI = input.at(i);
            if (charAtI.isNumber())
            {
                coefText.append(charAtI);
            }
            else
            {
                varText = input.mid(i, input.length()-i);
                varText = varText.trimmed();
                break;
            }
        }
        if (coefText!="")
            coef = coefText.toInt();
        else
            coef = 1;


        if (!mListVariable.contains(varText))
            mListVariable.append(varText);
    }


    void multiContainStar(QString input, int &coef, QString &varText)
    {
        QStringList resultAfterSplitStar = input.simplified().split("*");

    //    qDebug() << resultAfterSplitStar;
        foreach (QString str, resultAfterSplitStar)
        {
            str = str.trimmed();
            bool isOk = false;
            int value = str.toInt(&isOk);
            if (isOk)
            {
                coef *= value;
            }
            else
            {
                int tempCoef = 1;
                multiNotContainStar(str, tempCoef, varText);
                coef *= tempCoef;
                varText = varText.trimmed();
            }
        }

        if (!mListVariable.contains(varText))
            mListVariable.append(varText);
    }

    void multiplySolver(QTextStream &textStream, QString input = "")
    {
        bool isNumber = false;
        input.simplified().trimmed().toInt(&isNumber);
        if (isNumber)
        {
    //        qDebug() << "is number, not multiply exp";
            return;
        }

        if (input.length() == 1)
        {
    //        qDebug() << "multi with 1";
            if (!mListVariable.contains(input.simplified().trimmed()))
                mListVariable.push_back(input.simplified().trimmed());

            return;
        }

        int coef = 1;
        QString varText;
        if (input.contains("*"))
        {
            multiContainStar(input, coef, varText);
        }
        else
        {
            multiNotContainStar(input, coef, varText);
        }

        if (coef >=2)
        {
            QString sumLooping = "2"+varText;
            textStream << generateSumRule(varText,varText,sumLooping);
            for (int i = 1; i <= (coef - 2); i++)
            {
                QString oldSumLooping = sumLooping;
                sumLooping = QString::number(2+i)+varText;
                textStream << generateSumRule(oldSumLooping,varText,sumLooping);
            }
        }

    }

    void handlePlusAndMinus(QString input, QStringList& positiveVarList, QStringList& negativeVarList)
    {
        input = input.simplified().trimmed();
        QStringList listAfterRemovePlus = input.split("+");
        foreach (QString str, listAfterRemovePlus)
        {
            str = str.simplified().trimmed();
            if (str.contains("-"))
            {
                int startIndex = 0;
                for (startIndex; startIndex < str.length(); )
                {
                    int pos = startIndex;
                    bool isNegative = str.at(pos) == '-';
                    QString varText;
                    int startI = (isNegative ? (pos + 1) : pos);
                    for (int i = startI ; i < str.length(); i++)
                    {
                        if (str.at(i) == '-')
                        {
                            startIndex = i;
                            varText = str.mid(startI, startIndex - startI);
                            break;
                        }
                        if (i == str.length() - 1)
                        {
                            startIndex = str.length();
                            varText = str.mid(startI, startIndex - startI);
                            break;
                        }
                    }

                    if (isNegative)
                        negativeVarList.append(varText.simplified().trimmed());
                    else
                        positiveVarList.append(varText.simplified().trimmed());
                }
            }
            else
            {
                positiveVarList.append(str);
            }
        }

    //    qDebug() << "positiveList:"<<positiveVarList;
    //    qDebug() << "negativeList:"<<negativeVarList;
    }

    void generateNormalizationForm(QString input, QStringList& leftSide, QStringList& rightSide)
    {
        input = input.simplified().trimmed();

        QStringList listEqualRemoved = input.split("=");
        if (listEqualRemoved.count() > 1)
        {
            QString leftText = listEqualRemoved.at(0);
            QString rightText = listEqualRemoved.at(1);

            QStringList leftPositiveList, leftNegativeList;
            handlePlusAndMinus(leftText, leftPositiveList, leftNegativeList);

            QStringList rightPositiveList, rightNegativeList;
            handlePlusAndMinus(rightText, rightPositiveList, rightNegativeList);

            leftSide.append(leftPositiveList);
            leftSide.append(rightNegativeList);

            rightSide.append(rightPositiveList);
            rightSide.append(leftNegativeList);
        }
        else
        {
            qDebug() << "Exp incorrect!";
            return;
        }

        foreach (QString str, leftSide) {
            if (str == "0")
                leftSide.removeOne(str);
        }

        foreach (QString str, rightSide) {
            if (str == "0")
                rightSide.removeOne(str);
        }
    }



    QString sumOfSide(QTextStream& textStream, QStringList varsOnSide)
    {
        if (varsOnSide.isEmpty())
            return "0";
        else if (varsOnSide.count() == 1)
        {
            multiplySolver(textStream, varsOnSide.at(0));
            return varsOnSide.at(0);
        }
        else
        {
            QString sumTemp = varsOnSide.at(0);
            multiplySolver(textStream, sumTemp);

            for (int i = 1; i < varsOnSide.count(); i++)
            {
                QString varAtI = varsOnSide.at(i);
                multiplySolver(textStream, varAtI);

                QString oldSumTemp = sumTemp;
                sumTemp = oldSumTemp + "+" + varAtI;
                textStream << generateSumRule(oldSumTemp, varAtI, sumTemp);
            }
            return sumTemp;
        }
    }

    void expressionSolver(QTextStream& textStream, QString input = 0)
    {
        qDebug() << input;
        QStringList leftSideEles, rightSideEles;
        generateNormalizationForm(input, leftSideEles, rightSideEles);

        QString sumOfLeftSide = sumOfSide(textStream, leftSideEles);
        QString sumOfRightSide = sumOfSide(textStream, rightSideEles);

    //    textStream << generateSumRule(sumOfLeftSide, "0", sumOfRightSide);

        textStream << generateEqualRule(sumOfLeftSide, sumOfRightSide);
    }


    void reset();

    QStringList solve(QStringList listExpression);

private:
    QFile * ruleFlie;
    QTextStream * ruleStreamWrite;

    QString ruleFileName = "rule.txt";
    QString cnfFileName = "clause.cnf";
    int BIT_LENGTH = 8;
    int globalIndex = 1;
    QMap<int,QString> mapBitVariables;
    QStringList mListVariable;

};

#endif // SMTSOLVER_H
