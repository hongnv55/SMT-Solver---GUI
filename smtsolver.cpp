#include "smtsolver.h"
#include <QCoreApplication>

using namespace std;
using namespace Minisat;


//=================================================================================================


static Solver* solver;
// Terminate by notifying the solver and back out gracefully. This is mainly to have a test-case
// for this feature of the Solver as it may take longer than an immediate call to '_exit()'.
static void SIGINT_interrupt(int) { solver->interrupt(); }

// Note that '_exit()' rather than 'exit()' has to be used. The reason is that 'exit()' calls
// destructors and may cause deadlocks if a malloc/free function happens to be running (these
// functions are guarded by locks for multithreaded use).
static void SIGINT_exit(int) {
    printf("\n"); printf("*** INTERRUPTED ***\n");
    if (solver->verbosity > 0){
        solver->printStats();
        printf("\n"); printf("*** INTERRUPTED ***\n"); }
    _exit(1); }


//=================================================================================================


SMTSolver::SMTSolver()
{


    QString applicationPath = QCoreApplication::applicationDirPath();

    qDebug() << "application path:" << applicationPath;

    ruleFileName = "rule.txt";
    cnfFileName = "clause.cnf";
    BIT_LENGTH = 8;
    globalIndex = 1;
    mapBitVariables.clear();
    mListVariable.clear();


    ruleFlie = new QFile (ruleFileName);
    ruleStreamWrite = new QTextStream (ruleFlie);
}

void SMTSolver::runSATSolver(string resultFileName, SimpSolver &S)
{
    try {
            setUsageHelp("USAGE: %s [options] <input-file> <result-output-file>\n\n  where input may be either in plain or gzipped DIMACS.\n");
            setX86FPUPrecision();

            // Extra options:
            //
            IntOption    verb   ("MAIN", "verb",   "Verbosity level (0=silent, 1=some, 2=more).", 1, IntRange(0, 2));
            BoolOption   pre    ("MAIN", "pre",    "Completely turn on/off any preprocessing.", true);
            BoolOption   solve  ("MAIN", "solve",  "Completely turn on/off solving after preprocessing.", true);
            StringOption dimacs ("MAIN", "dimacs", "If given, stop after preprocessing and write the result to this file.");
            IntOption    cpu_lim("MAIN", "cpu-lim","Limit on CPU time allowed in seconds.\n", 0, IntRange(0, INT32_MAX));
            IntOption    mem_lim("MAIN", "mem-lim","Limit on memory usage in megabytes.\n", 0, IntRange(0, INT32_MAX));
            BoolOption   strictp("MAIN", "strict", "Validate DIMACS header during parsing.", false);

//            SimpSolver  S = simpleSolver;
            double      initial_time = cpuTime();

            if (!pre) S.eliminate(true);

            S.verbosity = verb;

            solver = &S;
            // Use signal handlers that forcibly quit until the solver will be able to respond to
            // interrupts:
            sigTerm(SIGINT_exit);

            // Try to set resource limits:
            if (cpu_lim != 0) limitTime(cpu_lim);
            if (mem_lim != 0) limitMemory(mem_lim);

            gzFile in = gzopen(cnfFileName.toStdString().c_str(), "rb");

            if (S.verbosity > 0){
                printf("============================[ Problem Statistics ]=============================\n");
                printf("|                                                                             |\n"); }

            parse_DIMACS(in, S, (bool)strictp);
            gzclose(in);
            FILE* res = fopen(resultFileName.c_str(), "wb");

            if (S.verbosity > 0){
                printf("|  Number of variables:  %12d                                         |\n", S.nVars());
                printf("|  Number of clauses:    %12d                                         |\n", S.nClauses()); }

            double parsed_time = cpuTime();
            if (S.verbosity > 0)
                printf("|  Parse time:           %12.2f s                                       |\n", parsed_time - initial_time);

            // Change to signal-handlers that will only notify the solver and allow it to terminate
            // voluntarily:
            sigTerm(SIGINT_interrupt);

            S.eliminate(true);
            double simplified_time = cpuTime();
            if (S.verbosity > 0){
                printf("|  Simplification time:  %12.2f s                                       |\n", simplified_time - parsed_time);
                printf("|                                                                             |\n"); }

            if (!S.okay()){
                if (res != NULL)
                {
                    fprintf(res, "UNSAT\n");
                    fclose(res);
                }
                if (S.verbosity > 0){
                    printf("===============================================================================\n");
                    printf("Solved by simplification\n");
                    S.printStats();
                    printf("\n"); }
                printf("UNSATISFIABLE\n");
//                exit(20);
            }

            lbool ret = l_Undef;

            if (solve){
                vec<Lit> dummy;
                ret = S.solveLimited(dummy);
            }else if (S.verbosity > 0)
                printf("===============================================================================\n");

            if (dimacs && ret == l_Undef)
                S.toDimacs((const char*)dimacs);

            if (S.verbosity > 0){
                S.printStats();
                printf("\n"); }
            printf(ret == l_True ? "SATISFIABLE\n" : ret == l_False ? "UNSATISFIABLE\n" : "INDETERMINATE\n");
            if (res != NULL){
                if (ret == l_True){
                    fprintf(res, "SAT\n");
                    for (int i = 0; i < S.nVars(); i++)
                        if (S.model[i] != l_Undef)
                            fprintf(res, "%s%s%d", (i==0)?"":" ", (S.model[i]==l_True)?"":"-", i+1);
                    fprintf(res, " 0\n");
                }else if (ret == l_False)
                    fprintf(res, "UNSAT\n");
                else
                    fprintf(res, "INDET\n");
                fclose(res);
            }

        } catch (OutOfMemoryException&){
            printf("===============================================================================\n");
            printf("INDETERMINATE\n");
//            exit(0);
    }
}

void SMTSolver::reset()
{
    ruleFileName = "rule.txt";
    cnfFileName = "clause.cnf";
    BIT_LENGTH = 8;
    globalIndex = 1;
    mapBitVariables.clear();
    mListVariable.clear();
    ruleFlie->close();
    ruleStreamWrite->reset();
}

QStringList SMTSolver::solve(QStringList listExpression)
{
    if (!ruleFlie->open(QIODevice::WriteOnly))
    {
//        qDebug() << "create " << ruleFileName << " now";
        return QStringList();
    }

    foreach (QString var, listExpression) {
//        qDebug() << "var = " << var;
        expressionSolver(*ruleStreamWrite, var);
    }
    ruleFlie->close();
    generateCnfFlie();

    SimpSolver simpleSolver;
    runSATSolver("result.txt", simpleSolver);

    QStringList resultExp;

    vec<Lit> dummy;
    lbool ret = simpleSolver.solveLimited(dummy);

    if (ret == l_False || ret == l_Undef)
    {
        resultExp.append("Cannot calculate!");
    }
    else
    {
        foreach (QString var, mListVariable) {
            QString result = "";
            for (int i = 0; i < BIT_LENGTH; i++)
            {
                int varSAT = mapBitVariables.key(QString("%1_%2").arg(var).arg(i));
                result.push_front(simpleSolver.model[varSAT - 1] == l_True ? "1" : "0");
            }
            bool test = false;
            resultExp.append(QString("%1 = %2\n").arg(var).arg(result.toInt(&test, 2)));
        }
    }

    reset();
    return resultExp;
}

