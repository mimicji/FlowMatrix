#include "VState.hpp"
#include "trace.hpp"

using namespace FlowMatrix;

int main()
{
    // Load FM trace
    trace_handle_t *trace_handle = Trace::LoadTrace("../../testsuite/ls_fmtrace/");

    // Init vstate with default settings
    VState *vstate = new VState(FMGranularity::byte);
    vstate->init_reg_state(FMRegListConfig_default);

    // Extend vstate with mem accesses in FM trace
    mem_ref_t mem_ref;
    while (trace_handle->mem_ref_set.ReadNext(mem_ref))
    {
        vstate->append(mem_ref.mem_ref_addr, 8);
    }

    // Print vstate
    vstate->pp();

    // Close FM trace
    Trace::CloseTrace(trace_handle);
}
