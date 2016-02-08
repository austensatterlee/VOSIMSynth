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
            Signal& outputChannel = port[i].connectedUnit->getOutputBus().getChannel(port[i].connectedPort);
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
        for(int i=0;i<port.size();i++){

            port[i].connectedUnit->tick();

            // add outputChannel to a_recipient
            Signal& inputChannel = port[i].connectedUnit->getInputBus().getChannel(port[i].connectedPort);
            inputChannel.set(inputChannel.get() + outputValue);
        }
        return true;
    }

    int UnitConnectionBus::numPorts() const
    {
        return m_ports.size();
    }
}
