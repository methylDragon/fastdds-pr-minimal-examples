// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file HelloWorldPublisher.cpp
 *
 */

#include "HelloWorldPublisher.h"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicDataHelper.hpp>

#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <thread>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::types;

HelloWorldPublisher::HelloWorldPublisher()
    : mp_participant(nullptr)
    , mp_publisher(nullptr)
{
}

bool HelloWorldPublisher::init()
{
    // Create type
    DynamicTypeBuilderFactory * factory = DynamicTypeBuilderFactory::get_instance();
    DynamicTypeBuilder * builder = factory->create_struct_builder();

    builder->add_member(0, "string_field", factory->create_string_type());
    builder->add_member(1, "bool_static_array_field",
                        factory->create_array_builder(factory->create_bool_type(), {5}));

    DynamicTypeBuilder * nested_builder = factory->create_struct_builder();
    nested_builder->add_member(0, "nested_bool_field", factory->create_bool_type());
    nested_builder->set_name("inner");

    builder->add_member(2, "nested_field", nested_builder->build());
    builder->set_name("HelloWorld");

    DynamicType_ptr dyn_type = builder->build();

    // Introspect type
    std::map<std::string, eprosima::fastrtps::types::DynamicTypeMember *> field_map;
    dyn_type->get_all_members_by_name(field_map);

    std::cout << "\nGENERATED TYPE:" << std::endl;
    for (auto const& x : field_map) {
        std::cout << x. first << ':' << x.second << std::endl;
    }
    std::cout << std::endl;

    // Populate type
    this->msg_data_ =
      eprosima::fastrtps::types::DynamicDataFactory::get_instance()->create_data(dyn_type);

    this->msg_data_ = DynamicDataFactory::get_instance()->create_data(dyn_type);

    this->msg_data_->set_string_value("A message!", 0);
    auto bool_array = this->msg_data_->loan_value(1);
    for (uint32_t i = 0; i < 5; ++i) {
      bool_array->set_bool_value(false, bool_array->get_array_index({i}));
    }
    this->msg_data_->return_loaned_value(bool_array);

    DomainParticipantQos pqos;
    pqos.name("Participant_pub");
    mp_participant = DomainParticipantFactory::get_instance()->create_participant(0, pqos);

    if (mp_participant == nullptr)
    {
        return false;
    }

    // Register type
    TypeSupport m_type(new eprosima::fastrtps::types::DynamicPubSubType(dyn_type));

    m_type.get()->auto_fill_type_information(false);
    m_type.get()->auto_fill_type_object(true);

    m_type.register_type(mp_participant);

    //CREATE THE PUBLISHER
    mp_publisher = mp_participant->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

    if (mp_publisher == nullptr)
    {
        return false;
    }

    topic_ = mp_participant->create_topic("DDSDynHelloWorldTopic", "HelloWorld", TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return false;
    }

    // CREATE THE WRITER
    writer_ = mp_publisher->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, &m_listener);

    if (writer_ == nullptr)
    {
        return false;
    }

    return true;

}

HelloWorldPublisher::~HelloWorldPublisher()
{
    if (writer_ != nullptr)
    {
        mp_publisher->delete_datawriter(writer_);
    }
    if (mp_publisher != nullptr)
    {
        mp_participant->delete_publisher(mp_publisher);
    }
    if (topic_ != nullptr)
    {
        mp_participant->delete_topic(topic_);
    }
    DomainParticipantFactory::get_instance()->delete_participant(mp_participant);
}

void HelloWorldPublisher::PubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        n_matched = info.total_count;
        firstConnected = true;
        std::cout << "Publisher matched" << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        n_matched = info.total_count;
        std::cout << "Publisher unmatched" << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void HelloWorldPublisher::runThread(
        uint32_t sleep)
{
  using namespace eprosima::fastrtps::types;

  while (!stop) {
    if (publish(true)) {
      std::cout << "\n== SENT ==" << std::endl;
      DynamicDataHelper::print(msg_data_);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
  }
}

void HelloWorldPublisher::run(
        uint32_t sleep)
{
  stop = false;
  std::thread thread(&HelloWorldPublisher::runThread, this, sleep);

  std::cout << "Publisher running. Please press enter to stop the Publisher at any time."
            << std::endl;
  std::cin.ignore();
  stop = true;

  thread.join();
}

bool HelloWorldPublisher::publish(
        bool waitForListener)
{
    if (m_listener.firstConnected || !waitForListener || m_listener.n_matched > 0)
    {
        bool bool_;
        eprosima::fastrtps::types::DynamicData * bool_array_ = msg_data_->loan_value(1);
        bool_array_->get_bool_value(bool_, bool_array_->get_array_index({0}));

        for (uint32_t i = 0; i < 5; i++) {
          bool_array_->set_bool_value(!bool_, bool_array_->get_array_index({i}));
        }
        msg_data_->return_loaned_value(bool_array_);

        eprosima::fastrtps::types::DynamicData * inner_ = msg_data_->loan_value(2);
        inner_->set_bool_value(!bool_, 0);
        msg_data_->return_loaned_value(inner_);

        writer_->write(msg_data_.get());
        return true;
    }
    return false;
}

int main(int argc, char * argv[])
{
  (void)argc;
  (void)argv;

  HelloWorldPublisher pub;
  if (pub.init()) {
    pub.run(1000);
  }

  return 0;
}
