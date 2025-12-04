#!/usr/bin/python3

import os
import sys
from mininet.topo import Topo
from mininet.net import Mininet
from mininet.cli import CLI
from mininet.log import setLogLevel


def cleanup_mininet():
    os.system('sudo mn -c >/dev/null 2>&1')


class LeafSpineTopo(Topo):
    def __init__(self, leaf_radix=4, spine_radix=4):
        leaf_count = leaf_radix - 2
        spine_count = spine_radix

        if leaf_count <= 0 or spine_count <= 0:
            raise ValueError("Invalid radix values.")

        super().__init__()

        self.leaf_switches = [self.addSwitch(f"l{i+1}") for i in range(leaf_count)]
        self.spine_switches = [self.addSwitch(f"s{i+1}") for i in range(spine_count)]

        for leaf in self.leaf_switches:
            for spine in self.spine_switches:
                self.addLink(leaf, spine)

        host_id = 1
        for leaf in self.leaf_switches:
            for _ in range(2):
                host = self.addHost(f"h{host_id}")
                self.addLink(host, leaf)
                host_id += 1


def start_network(topo: Topo):
    net = None
    try:
        net = Mininet(topo=topo)
        net.start()
    except Exception:
        if net:
            net.stop()
        try:
            net = Mininet(topo=topo, controller=None)
            net.start()
        except Exception:
            if net:
                net.stop()
            return None
    return net


def create_leaf_spine(radix=4):
    cleanup_mininet()
    topo = LeafSpineTopo(leaf_radix=radix, spine_radix=radix)
    net = start_network(topo)
    if not net:
        print("Failed to start network. Please check Mininet installation.")
        return

    CLI(net)
    net.stop()


if __name__ == "__main__":
    setLogLevel("info")
    radix = 4
    if len(sys.argv) > 1:
        try:
            radix = int(sys.argv[1])
        except ValueError:
            pass
    create_leaf_spine(radix)