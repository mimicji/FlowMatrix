#include "VState.hpp"

using namespace FlowMatrix;

int main()
{
    std::cout << "Bit vstate: " << std::endl;
    VState *vstate_bit = new VState(FMGranularity::bit);
    vstate_bit->init_reg_state(FMRegListConfig_default);
    vstate_bit->pp();
    delete vstate_bit;

    std::cout << "\nByte vstate: " << std::endl;
    VState *vstate_byte = new VState(FMGranularity::byte);
    vstate_byte->init_reg_state(FMRegListConfig_default);
    vstate_byte->pp();
    delete vstate_byte;

    std::cout << "\nQword vstate: " << std::endl;
    VState *vstate_qword = new VState(FMGranularity::qword);
    vstate_qword->init_reg_state(FMRegListConfig_default);
    vstate_qword->pp();
    delete vstate_qword;
}