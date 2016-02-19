/// 
/// Created by austen on 2/6/2016.
///

#ifndef VOSIMLIB_NAMEDCONTAINER_H
#define VOSIMLIB_NAMEDCONTAINER_H
#include <vector>
#include <string>
#include <stdexcept>

using std::vector;
using std::pair;
using std::make_pair;
using std::string;

namespace syn {

    template<typename T>
    class NamedContainer {
    public:
        virtual ~NamedContainer(){ m_names.clear(); m_data.clear(); }
        /**
         * Add a named item to the container.
         * \returns The items index in the container, or -1 if the item or name already exists.
         */
        int add(const string& a_name, const T& a_item);

        /**
         * Remove an item from the container, either by its name, index, or a reference to the item itself.
         * \returns True upon successful removal, false if item does not exist.
         */
        template<typename ID>
        bool remove(const ID& a_itemID);

        /**
         * Retrieves an item from the container, either by its name, index, or a reference to the item itself.
         * \returns A reference to the item
         */
        template<typename ID>
        T& operator[](const ID& a_itemID);

        template<typename ID>
        const T& operator[](const ID& a_itemID) const;

        /**
         * Verifies if an item is in the container, either by its name, index, or a reference to the item itself.
         * \returns True if the item exists, false otherwise.
         */
        template<typename ID>
        bool find(const ID& a_itemID) const;

        template<typename ID>
        string getItemName(const ID& a_itemID) const;

        size_t size() const;

	    static int getItemIndex(int a_itemIndex);

        int getItemIndex(const string& a_name) const;

        int getItemIndex(const T& a_item) const;

    private:
        vector<T> m_data;
        vector<pair<string, int> > m_names;
    };

    template<typename T>
    int NamedContainer<T>::add(const string& a_name, const T& a_item){
        if( find(a_name) ){
            return -1;
        }
        int item_index = m_data.size();
        m_data.push_back(a_item);
        m_names.push_back(make_pair(a_name, item_index));
        return item_index;
    }

    template<typename T>
    template<typename ID>
    bool NamedContainer<T>::remove(const ID& a_itemID){
        int itemidx = getItemIndex(a_itemID);
        if(itemidx<0){
            return false;
        }
        // Check if item was named and, if so, remove its name from the list
        for(unsigned i=0;i<m_names.size();i++){
            if(m_names[i].second==itemidx){
                m_names.erase(m_names.begin()+i);
                i--;
            }else if(m_names[i].second>itemidx){
                m_names[i].second--;
            }
        }
        m_data.erase(m_data.begin() + itemidx);
        return true;
    }

    template<typename T>
    template<typename ID>
    T& NamedContainer<T>::operator[](const ID& a_itemID) {
        int itemidx = getItemIndex(a_itemID);
        if(itemidx<0){
            throw std::logic_error("requested item does not exist");
        }else{
            return m_data[itemidx];
        }
    }

    template<typename T>
    template<typename ID>
    const T& NamedContainer<T>::operator[](const ID& a_itemID) const {
        int itemidx = getItemIndex(a_itemID);
        if(itemidx<0){
            throw std::logic_error("requested item does not exist");
        }else{
            return m_data[itemidx];
        }
    }

    template<typename T>
    template<typename ID>
    bool NamedContainer<T>::find(const ID& a_itemID) const{
        return (getItemIndex(a_itemID) >= 0);
    }

    template<typename T>
    size_t NamedContainer<T>::size() const{
        return m_data.size();
    }

    template<typename T>
    template<typename ID>
    string NamedContainer<T>::getItemName(const ID& a_itemID) const{
        int itemidx = getItemIndex(a_itemID);
        if(itemidx<0){
            return "";
        }
        // Check if item was named and, if so, remove its name from the list
        for(int i=0;i<m_names.size();i++){
            if(m_names[i].second==itemidx){
                return m_names[i].first;
            }
        }
		throw std::domain_error("Item does not exist in NamedContainer");
    }

    template<typename T>
    int NamedContainer<T>::getItemIndex(int a_itemIndex) {
		return a_itemIndex;
    }

    template<typename T>
    int NamedContainer<T>::getItemIndex(const string& a_name) const{
		int nNames = m_names.size();
        for(int i=0;i<nNames;i++){
            if(m_names[i].first == a_name){
                return m_names[i].second;
            }
        }
        return -1;
    }

    template<typename T>
    int NamedContainer<T>::getItemIndex(const T& a_item) const{
		int nNames = m_names.size();
        for(int i=0;i<nNames;i++){
            int itemidx = m_names[i].second;
            if(m_data[itemidx] == a_item){
                return itemidx;
            }
        }
        return -1;
    }
}


#endif //VOSIMLIB_NAMEDCONTAINER_H
