#include <cli/cli.h>
#include <cli/clifilesession.h>
#include <FMQuery.hpp>
#include <QueryCLI.hpp>
#include <SyscallHook.hpp>
#include <unistd.h>

using namespace FlowMatrix;

void printUsage(char **argv)
{
    printf("Usage: %s <StartRange> <EndRange> <TraceName> <PathToDB> <StorageLevel>\n", argv[0]);
}

int main(int argc, char **argv)
{
    int pid = getpid();

    if (argc != 6)
    {
        printUsage(argv);
        exit(0);
    }

    // Init DB
    std::string traceName(argv[3]);
    std::string dBPath(argv[4]);
    DBCore *db = new DBCore(dBPath, traceName);

    // Init Query Handle
    int start = atoi(argv[1]);
    int end = atoi(argv[2]);
    int storageLevel = atoi(argv[5]);

    LOG("%d: Prepare query tree on range (%d, %d)", pid, start, end);
    FMQuery *query = new FMQuery(dBPath, traceName, end, start, end);
    query->BuildTree(storageLevel, 1, true);

#ifdef TIME_MEASUREMENT
    query->sp_core->printTimer();
    db->printTimer();
#endif

    delete query;
    delete db;
}