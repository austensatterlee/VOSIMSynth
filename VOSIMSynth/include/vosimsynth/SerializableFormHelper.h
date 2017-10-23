// /*
// This file is part of VOSIMProject.
// 
// VOSIMProject is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// VOSIMProject is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with VOSIMProject.  If not, see <http://www.gnu.org/licenses/>.
// 
// Copyright 2016, Austen Satterlee
// */

#pragma once
#include <nanogui/formhelper.h>

namespace synui {
    using nanogui::FormHelper;
    using nanogui::detail::FormWidget;

    /**
     * \brief Performs the same function as \ref FormHelper, but provides automatic serialization for its fields.
     */
    class SerializableFormHelper : public FormHelper {
    public:
        explicit SerializableFormHelper(nanogui::Screen* screen);

        /**
        * \brief Adds a variable that will be automatically serialized with this form.
        *
        * \param a_name A name that will be used to serialize this variable. Will (ideally) never change.
        * \param a_label The label for this variable that will be displayed in the gui.
        */
        template <typename Type>
        FormWidget<Type>* addSerializableVariable(const std::string& a_name, const std::string& a_label,
                                                  const std::function<void(const Type&)>& a_setter,
                                                  const std::function<Type()>& a_getter,
                                                  bool a_editable = true);

        template <typename Type>
        FormWidget<Type>* addSerializableVariable(const std::string& a_label,
                                                  const std::function<void(const Type&)>& a_setter,
                                                  const std::function<Type()>& a_getter,
                                                  bool a_editable = true);

        operator nlohmann::json() const;

        SerializableFormHelper* load(const nlohmann::json& j);

    protected:
        std::map<std::string, std::function<nlohmann::json()>> m_getterSerializers;
        std::map<std::string, std::function<void(const nlohmann::json&)>> m_setterSerializers;
    };

    inline SerializableFormHelper::SerializableFormHelper(nanogui::Screen* screen)
        : FormHelper(screen) { }

    template <typename Type>
    FormWidget<Type>* SerializableFormHelper::addSerializableVariable(const std::string& a_name,
                                                                      const std::string& a_label,
                                                                      const std::function<void(const Type&)>& a_setter,
                                                                      const std::function<Type()>& a_getter,
                                                                      bool a_editable) {
        auto ret = FormHelper::addVariable<Type>(a_label, a_setter, a_getter, a_editable);
        auto getterSerializer = [a_getter]()
        {
            nlohmann::json j = a_getter();
            return j;
        };
        auto setterSerializer = [a_setter](const nlohmann::json& j)
        {
            a_setter(j.get<Type>());
        };
        m_getterSerializers[a_name] = getterSerializer;
        m_setterSerializers[a_name] = setterSerializer;
        return ret;
    }

    template <typename Type>
    FormWidget<Type>* SerializableFormHelper::addSerializableVariable(const std::string& a_label,
                                                                      const std::function<void(const Type&)>& a_setter,
                                                                      const std::function<Type()>& a_getter,
                                                                      bool a_editable) {
        return addSerializableVariable<Type>(a_label, a_label, a_setter, a_getter, a_editable);
    }

    inline SerializableFormHelper::operator nlohmann::basic_json<>() const {
        nlohmann::json j;
        for (auto& g : m_getterSerializers) {
            j[g.first] = g.second();
        }
        return j;
    }

    inline SerializableFormHelper* SerializableFormHelper::load(const nlohmann::json& j) {
        for (auto& s : m_setterSerializers) {
            const nlohmann::json& curr = j[s.first];
            if (!curr.empty())
                s.second(curr);
        }
        return this;
    }
}
