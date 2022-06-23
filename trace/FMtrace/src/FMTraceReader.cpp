#include <trace.hpp>

bool print_instr = false;
bool print_syscall = false;
bool print_mem_ref_set = false;

void print_usage(const char* program_name)
{
    fprintf(stderr, "Usage: %s [Options] trace_name\n", program_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -s               \tPrint syscall trace.\n");
    fprintf(stderr, "  -i               \tPrint instruction trace.\n");
    fprintf(stderr, "  -m               \tPrint mem access address set.\n");
    fprintf(stderr, "  -h               \tPrint this help.\n");
}

char *parse_input(int argc, char *argv[])
{
    // Argument parsing
    int opt;
    while ((opt = getopt(argc, argv, "hism")) != -1) {
        switch (opt) {
        case 's':
            print_syscall = true;
            break;
        case 'i':
            print_instr = true;
            break;
        case 'm':
            print_mem_ref_set = true;
            break;
        case 'h':
            print_usage(argv[0]);
            exit(EXIT_FAILURE);
            break;
        }
    }
    if (optind >= argc) 
    {
        print_usage(argv[0]);
        fprintf(stderr, "\nMissing argument: Trace path at the end expected.\n");
        exit(EXIT_FAILURE);
    }

    return argv[argc - 1];
}

int main(int argc, char *argv[])
{
    std::string trace_name(parse_input(argc, argv));
    FlowMatrix::trace_handle_t *handle = FlowMatrix::Trace::LoadTrace(trace_name);

    if (print_instr)
    {
        FlowMatrix::instr_t instr;
        while (handle->instr_trace.ReadNext(instr))
        {
            instr.Print();
        }
    }

    if (print_syscall)
    {
        FlowMatrix::syscall_t syscall;
        while (handle->syscall_trace.ReadNext(syscall))
            syscall.Print();      
    }

    if (print_mem_ref_set)
    {
        FlowMatrix::mem_ref_t mem_ref;
        while (handle->mem_ref_set.ReadNext(mem_ref))
            mem_ref.Print();      
    }

    FlowMatrix::Trace::CloseTrace(handle);
}