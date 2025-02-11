#include "ns3/device-client-factory.h"
#include "ns3/device-client-helper.h"
#include "ns3/callback.h"

namespace ns3 {

DeviceClientFactory::DeviceClientFactory(
    Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology, 
    DeviceClient::MessageSendCallbackType send_callback,
    DeviceClient::MessageReceiveCallbackType receive_callback
) {
    printf("DEVICE CLIENT FACTORY\n");

    m_basicSimulation = basicSimulation;
    m_topology = topology;

    // Check if it is enabled explicitly
    m_enabled = parse_boolean(m_basicSimulation->GetConfigParamOrDefault("enable_device_clients", "false"));
    if (!m_enabled) {
        std::cout << "  > Not enabled explicitly, so disabled" << std::endl;

    } else {
        std::cout << "  > Device client factory enabled" << std::endl;

        m_nodes = m_topology->GetNodes();
        m_simulation_end_time_ns = m_basicSimulation->GetSimulationEndTimeNs();
        m_interval_ns = parse_positive_int64(basicSimulation->GetConfigParamOrFail("send_devices_interval_ns"));
        std::string send_endpoints_pair_str = basicSimulation->GetConfigParamOrFail("send_devices_endpoint_pairs");
        m_system_id = m_basicSimulation->GetSystemId();
        m_enable_distributed = m_basicSimulation->IsDistributedEnabled();
        m_distributed_node_system_id_assignment = m_basicSimulation->GetDistributedNodeSystemIdAssignment();

        // Pairs
        std::set<std::string> string_set = parse_set_string(send_endpoints_pair_str);
        for (std::string s : string_set) {
            std::vector<std::string> spl = split_string(s, "->", 2);
            int64_t a = parse_positive_int64(spl[0]);
            int64_t b = parse_positive_int64(spl[1]);
            if (a == b) {
                throw std::invalid_argument(format_string("Cannot connect pair to itself on node %" PRIu64 "", a));
            }
            if (!m_topology->IsValidEndpoint(a)) {
                throw std::invalid_argument(format_string("Left node identifier in device pair is not a valid endpoint: %" PRIu64 "", a));
            }
            if (!m_topology->IsValidEndpoint(b)) {
                throw std::invalid_argument(format_string("Right node identifier in device pair is not a valid endpoint: %" PRIu64 "", b));
            }
            if (!m_enable_distributed || m_distributed_node_system_id_assignment[a] == m_system_id) {
                m_endpoint_pairs.push_back(std::make_pair(a, b));
            }
        }

        // Sort the pairs ascending such that we can do some spacing
        std::sort(m_endpoint_pairs.begin(), m_endpoint_pairs.end());

        // Schedule read
        printf("  > Determined device pairs (size: %lu)\n", m_endpoint_pairs.size());
        m_basicSimulation->RegisterTimestamp("Determined device pairs");

        // Determine filenames
        if (m_enable_distributed) {
            m_device_csv_filename = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_system_id) + "_send_device.csv";
            m_device_txt_filename = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_system_id) + "_send_device.txt";
        } else {
            m_device_csv_filename = m_basicSimulation->GetLogsDir() + "/send_device.csv";
            m_device_txt_filename = m_basicSimulation->GetLogsDir() + "/send_device.txt";
        }

        // Remove files if they are there
        remove_file_if_exists(m_device_csv_filename);
        remove_file_if_exists(m_device_txt_filename);
        printf("  > Removed previous log files if present\n");
        m_basicSimulation->RegisterTimestamp("Remove previous log files");

        // Info
        std::cout << "  > Send interval: " << m_interval_ns << " ns" << std::endl;

        // Endpoints
        std::set<int64_t> endpoints = m_topology->GetEndpoints();

        // Install echo client from each node to each other node
        std::cout << "  > Setting up " << m_endpoint_pairs.size() << " device clients" << std::endl;
        int64_t in_between_ns = m_interval_ns / (endpoints.size() - 1);
        int counter = 0;
        int64_t prev_i = -1;
        for (std::pair<int64_t, int64_t>& p : m_endpoint_pairs) {

            if (p.first != prev_i) {
                prev_i = p.first;
                counter = 0;
            }

            // Helper to install the source application
            DeviceClientHelper source(
                    m_nodes.Get(p.second)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(),
                    1025,
                    p.first,
                    p.second,
                    send_callback,
                    receive_callback
            );
            source.SetAttribute("Interval", TimeValue(NanoSeconds(m_interval_ns)));

            // Install it on the node and start it right now
            ApplicationContainer app = source.Install(m_nodes.Get(p.first));
            app.Start(NanoSeconds(counter * in_between_ns));
            m_apps.push_back(app);

            counter++;
        }
        m_basicSimulation->RegisterTimestamp("Setup device clients");

    }

    std::cout << std::endl;
}

void DeviceClientFactory::WriteResults() {
    std::cout << "STORE DEVICE CLIENT RESULTS" << std::endl;

    // Check if it is enabled explicitly
    if (!m_enabled) {
        std::cout << "  > Not enabled, so no device results are written" << std::endl;

    } else {

        // Open files
        std::cout << "  > Opening log files:" << std::endl;
        FILE* file_csv = fopen(m_device_csv_filename.c_str(), "w+");
        std::cout << "    >> Opened: " << m_device_csv_filename << std::endl;
        FILE* file_txt = fopen(m_device_txt_filename.c_str(), "w+");
        std::cout << "    >> Opened: " << m_device_txt_filename << std::endl;

        // Header
        std::cout << "  > Writing " << m_device_txt_filename << std::endl;
        fprintf(file_txt, "%-10s%-10s%-22s%-22s%-16s%-16s%-16s%-16s%s\n",
                "Source", "Target", "Mean latency there", "Mean latency back",
                "Min. RTT", "Mean RTT", "Max. RTT", "Smp.std. RTT", "Reply arrival");

        // Go over the applications, write each device's result
        for (uint32_t i = 0; i < m_apps.size(); i++) {
            Ptr<DeviceClient> client = m_apps[i].Get(0)->GetObject<DeviceClient>();

            // Data about this pair
            int64_t from_node_id = client->GetFromNodeId();
            int64_t to_node_id = client->GetToNodeId();
            uint32_t sent = client->GetSent();
            std::vector<int64_t> sendRequestTimestamps = client->GetSendRequestTimestamps();
            std::vector<int64_t> replyTimestamps = client->GetReplyTimestamps();
            std::vector<int64_t> receiveReplyTimestamps = client->GetReceiveReplyTimestamps();

            int total = 0;
            double sum_latency_to_there_ns = 0.0;
            double sum_latency_from_there_ns = 0.0;
            std::vector<int64_t> rtts_ns;
            for (uint32_t j = 0; j < sent; j++) {

                // Outcome
                bool reply_arrived = replyTimestamps[j] != -1;
                std::string reply_arrived_str = reply_arrived ? "YES" : "LOST";

                // Latencies
                int64_t latency_to_there_ns = reply_arrived ? replyTimestamps[j] - sendRequestTimestamps[j] : -1;
                int64_t latency_from_there_ns = reply_arrived ? receiveReplyTimestamps[j] - replyTimestamps[j] : -1;
                int64_t rtt_ns = reply_arrived ? latency_to_there_ns + latency_from_there_ns : -1;

                // Write plain to the csv
                fprintf(
                        file_csv,
                        "%" PRId64 ",%" PRId64 ",%u,%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%s\n",
                        from_node_id, to_node_id, j, sendRequestTimestamps[j], replyTimestamps[j], receiveReplyTimestamps[j],
                        latency_to_there_ns, latency_from_there_ns, rtt_ns, reply_arrived_str.c_str()
                );

                // Add to statistics
                if (reply_arrived) {
                    total++;
                    sum_latency_to_there_ns += latency_to_there_ns;
                    sum_latency_from_there_ns += latency_from_there_ns;
                    rtts_ns.push_back(rtt_ns);
                }

            }

            // Finalize the statistics
            int64_t min_rtt_ns = 10000000000000; // 10000s should be sufficiently high
            int64_t max_rtt_ns = -1;
            double mean_rtt_ns = (sum_latency_to_there_ns + sum_latency_from_there_ns) / total;
            double sum_sq = 0.0;
            for (uint32_t j = 0; j < rtts_ns.size(); j++) {
                min_rtt_ns = std::min(min_rtt_ns, rtts_ns[j]);
                max_rtt_ns = std::max(max_rtt_ns, rtts_ns[j]);
                sum_sq += std::pow(rtts_ns[j] - mean_rtt_ns, 2);
            }
            double sample_std_rtt_ns;
            double mean_latency_to_there_ns;
            double mean_latency_from_there_ns;
            if (rtts_ns.size() == 0) { // If no measurements came through, it should all be -1
                mean_latency_to_there_ns = -1;
                mean_latency_from_there_ns = -1;
                min_rtt_ns = -1;
                max_rtt_ns = -1;
                mean_rtt_ns = -1;
                sample_std_rtt_ns = -1;
            } else {
                mean_latency_to_there_ns = sum_latency_to_there_ns / total;
                mean_latency_from_there_ns = sum_latency_from_there_ns / total;
                sample_std_rtt_ns = rtts_ns.size() > 1 ? std::sqrt((1.0 / (rtts_ns.size() - 1)) * sum_sq) : 0.0;
            }

            // Write nicely formatted to the text
            char str_latency_to_there_ms[100];
            sprintf(str_latency_to_there_ms, "%.2f ms", nanosec_to_millisec(mean_latency_to_there_ns));
            char str_latency_from_there_ms[100];
            sprintf(str_latency_from_there_ms, "%.2f ms", nanosec_to_millisec(mean_latency_from_there_ns));
            char str_min_rtt_ms[100];
            sprintf(str_min_rtt_ms, "%.2f ms", nanosec_to_millisec(min_rtt_ns));
            char str_mean_rtt_ms[100];
            sprintf(str_mean_rtt_ms, "%.2f ms", nanosec_to_millisec(mean_rtt_ns));
            char str_max_rtt_ms[100];
            sprintf(str_max_rtt_ms, "%.2f ms", nanosec_to_millisec(max_rtt_ns));
            char str_sample_std_rtt_ms[100];
            sprintf(str_sample_std_rtt_ms, "%.2f ms", nanosec_to_millisec(sample_std_rtt_ns));
            fprintf(
                    file_txt, "%-10" PRId64 "%-10" PRId64 "%-22s%-22s%-16s%-16s%-16s%-16s%d/%d (%d%%)\n",
                    from_node_id, to_node_id, str_latency_to_there_ms, str_latency_from_there_ms, str_min_rtt_ms, str_mean_rtt_ms, str_max_rtt_ms, str_sample_std_rtt_ms, total, sent, (int) std::round(((double) total / (double) sent) * 100.0)
            );

        }

        // Close files
        std::cout << "  > Closing device log files:" << std::endl;
        fclose(file_csv);
        std::cout << "    >> Closed: " << m_device_csv_filename << std::endl;
        fclose(file_txt);
        std::cout << "    >> Closed: " << m_device_txt_filename << std::endl;

        // Register completion
        std::cout << "  > device log files have been written" << std::endl;
        m_basicSimulation->RegisterTimestamp("Write device log files");

    }

    std::cout << std::endl;
}

}
