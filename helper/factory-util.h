#ifndef FACTORY_UTIL_H
#define FACTORY_UTIL_H

#include "ns3/object.h"
#include "ns3/topology.h"

namespace ns3 {

    /// @brief Parse a string of form "A->B,X->Y" into a vector of pairs [(A,B), (X,Y)]
    std::vector<std::pair<int64_t, int64_t>> 
    parse_endpoint_pairs(
        const std::string& endpoints_pair_str, 
        Ptr<Topology> topology
    );

    /// @brief Parse a time unit string into Time::Unit
    Time::Unit
    parse_time_unit(const std::string& time_unit_str);

}

#endif // FACTORY_UTIL_H