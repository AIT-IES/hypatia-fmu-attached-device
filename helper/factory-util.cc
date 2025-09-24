#include "factory-util.h"
#include "ns3/exp-util.h"

using namespace std;

namespace ns3 {

vector<pair<int64_t, int64_t>>
parse_endpoint_pairs(
    const string& endpoints_pair_str,
    Ptr<Topology> topology
) {
    std::vector<std::pair<int64_t, int64_t>> endpoint_pairs;
    std::set<std::string> string_set = parse_set_string(endpoints_pair_str);
    for (std::string s : string_set) {
        std::vector<std::string> spl = split_string(s, "->", 2);
        int64_t a = parse_positive_int64(spl[0]);
        int64_t b = parse_positive_int64(spl[1]);
        if (a == b) {
            throw std::invalid_argument(format_string(
                "Cannot connect pair to itself on node %" PRIu64 "", a));
        } else if (!topology->IsValidEndpoint(a)) {
            throw std::invalid_argument(format_string(
                "Left node identifier in device pair is not a valid endpoint: %" PRIu64 "", a
            ));
        } else if (!topology->IsValidEndpoint(b)) {
            throw std::invalid_argument(format_string(
                "Right node identifier in device pair is not a valid endpoint: %" PRIu64 "", b
            ));
        }

        endpoint_pairs.push_back(std::make_pair(a, b));
    }

    return endpoint_pairs;
}

}
