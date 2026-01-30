/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/yans-wifi-helper.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ThirdScript_Lab2.1");

int
main(int argc, char* argv[])
{
    bool verbose = true;
    bool tracing = true;
    uint32_t nWifi = 5;
    NodeContainer wifiNodes;
    wifiNodes.Create(nWifi);
    

    CommandLine cmd(__FILE__);
    cmd.AddValue("nWifi", "Number of wifi devices", nWifi);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
    cmd.AddValue("tracing", "Enable pcap tracing", tracing);

    cmd.Parse(argc, argv);

    if (nWifi > 18)
    {
        std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box"
                  << std::endl;
        return 1;
    }

    if (verbose)
    {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());
     
    WifiMacHelper mac;
    //Ssid ssid = Ssid("ns-3-ssid");

    WifiHelper wifi;
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "RtsCtsThreshold", UintegerValue(0));
    wifi.SetStandard(WIFI_STANDARD_80211ac);
    
    NetDeviceContainer wifiDevices;
    mac.SetType("ns3::AdhocWifiMac");
    wifiDevices = wifi.Install(phy, mac, wifiNodes);

    MobilityHelper mobility;


    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX",
                                  DoubleValue(0.0),
                                  "MinY",
                                  DoubleValue(0.0),
                                  "DeltaX",
                                  DoubleValue(5.0),
                                  "DeltaY",
                                  DoubleValue(10.0),
                                  "GridWidth",
                                  UintegerValue(3),
                                  "LayoutType",
                                  StringValue("RowFirst"));

    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds",
                              RectangleValue(Rectangle(-90, 90, -90, 90)));  // Updated for assignment
    mobility.Install(wifiNodes);
    
    //mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    InternetStackHelper stack;
    stack.Install(wifiNodes);

    Ipv4AddressHelper address;

    address.SetBase("192.168.1.0", "255.255.255.0");
    //address.Assign(wifiDevices);
    Ipv4InterfaceContainer wifiInterfaces;
    wifiInterfaces = address.Assign(wifiDevices);

    // SERVER
    UdpEchoServerHelper echoServer(20);
    ApplicationContainer serverApp = echoServer.Install(wifiNodes.Get(0));
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    // CLIENTS
    
    // UDP Echo client on Node 3
    UdpEchoClientHelper echoClient_n3(wifiInterfaces.GetAddress(0), 20);
    echoClient_n3.SetAttribute("MaxPackets", UintegerValue(2));
    echoClient_n3.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient_n3.SetAttribute("PacketSize", UintegerValue(512));
    ApplicationContainer clientApps_n3 = echoClient_n3.Install(wifiNodes.Get(3));
    clientApps_n3.Start(Seconds(1.0));
    clientApps_n3.Stop(Seconds(3.0));
    
    // UDP Echo client on Node 4
    UdpEchoClientHelper echoClient_n4(wifiInterfaces.GetAddress(0), 20);
    echoClient_n4.SetAttribute("MaxPackets", UintegerValue(2));
    echoClient_n4.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient_n4.SetAttribute("PacketSize", UintegerValue(512));
    ApplicationContainer clientApps_n4 = echoClient_n4.Install(wifiNodes.Get(4));
    clientApps_n4.Start(Seconds(2.0));
    clientApps_n4.Stop(Seconds(5.0));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Simulator::Stop(Seconds(10.0));

    if (tracing)
    {
        phy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
        phy.EnablePcap("third_1_rts", wifiDevices.Get(1));
    }

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}



