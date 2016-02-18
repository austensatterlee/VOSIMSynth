/// 
/// Created by austen on 2/5/2016.
///

#include "UnitConnectionBus.h"
#include "Unit.h"

namespace syn {

    bool UnitConnectionBus::connect(shared_ptr<Unit> a_connectedUnit, int a_connectedPort, int a_localPort)
    {
        UnitConnector conn{a_connectedUnit, a_connectedPort, a_localPort};
        if(m_ports.size() <= a_localPort){
            m_ports.resize(a_localPort + 1);
        }

        /* Make sure we don't add duplicate connections */
        UnitPort& port = m_ports[a_localPort];
        for(int i=0;i<port.size();i++){
            if(port[i]==conn){
                return false;
            }
        }
        port.push_back(conn);
        return true;
    }

    bool UnitConnectionBus::disconnect(shared_ptr<Unit> a_connectedUnit, int a_connectedPort, int a_localPort)
    {
        UnitConnector conn{a_connectedUnit, a_connectedPort, a_localPort};
        if(m_ports.size() <= a_localPort){
            return false;
        }

        /* Search for connection in input port */
        UnitPort& port = m_ports[a_localPort];
        for(int i=0;i<port.size();i++){
            if(port[i]==conn){
                port.erase(port.begin()+i);
                return true;
            }
        }
        return false;
    }

    bool UnitConnectionBus::pull(int a_localPort, Signal& a_recipient) const
    {
        if(m_ports.size() <= a_localPort){
            return false;
        }
        const UnitPort& port = m_ports[a_localPort];
        double newInputValue = a_recipient.get();
        for(int i=0;i<port.size();i++){

            port[i].connectedUnit->tick();

            // add outputChannel to a_recipient
            const Signal& outputChannel = port[i].connectedUnit->getOutputChannel(port[i].connectedPort);
            newInputValue += outputChannel.get();
        }
        a_recipient.set(newInputValue);
        return true;
    }

    bool UnitConnectionBus::push(int a_localPort, Signal& a_signal) const
    {
        if(m_ports.size() <= a_localPort){
            return false;
        }
        const UnitPort& port = m_ports[a_localPort];
        double outputValue = a_signal.get();
        double inputValue;
        for(int i=0;i<port.size();i++){
            // add outputChannel to a_recipient
            inputValue = port[i].connectedUnit->getInputChannel(port[i].connectedPort).get();
            port[i].connectedUnit->setInputChannel(port[i].connectedPort, inputValue + outputValue);
        }
        return true;
    }

    int UnitConnectionBus::numPorts() const
    {
        return m_ports.size();
    }

    bool UnitConnectionBus::disconnect(shared_ptr<Unit> a_connectedUnit)
    {
        vector<pair<int,int> > garbage_list;
        // Find all connections referencing this unit and add them to the garbage list
        for(int i=0;i<m_ports.size();i++){
            for(int j=0;j<m_ports[i].size();j++){
                if(m_ports[i][j].connectedUnit == a_connectedUnit){
                    garbage_list.push_back(make_pair(i,j));
                }
            }
        }
        // Remove all connections in the garbage list
        for(int i=0;i<garbage_list.size();i++){
            UnitPort& port = m_ports[garbage_list[i].first];
            port.erase(port.begin()+garbage_list[i].second);
        }

        return garbage_list.size()>0;
    }
}
