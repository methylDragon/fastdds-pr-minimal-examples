#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>

#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>

#include <fastrtps/types/DynamicDataHelper.hpp>
#include <fastrtps/types/DynamicDataFactory.h>
#include <fastrtps/types/DynamicTypeBuilderFactory.h>
#include <fastrtps/types/DynamicTypeBuilderPtr.h>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::types;


int main(int argc, char * argv[])
{
  (void)argc;
  (void)argv;

  auto type_factory = DynamicTypeBuilderFactory::get_instance();
  auto data_factory = DynamicDataFactory::get_instance();

  DynamicTypeBuilder_ptr outer_builder(
    type_factory->create_struct_builder());
  DynamicTypeBuilder_ptr inner_builder(
    type_factory->create_struct_builder());


  // BUILD TYPES ===================================================================================
  // We're creating a struct type with a bounded sequence of inner structs
  inner_builder->add_member(0, "inner_uint32", type_factory->create_uint32_type());
  inner_builder->set_name("inner");
  auto inner_type = inner_builder->build();

  outer_builder->add_member(
    0, "nested_sequence", type_factory->create_sequence_builder(inner_type, 2)
  );
  outer_builder->set_name("outer");

  auto outer_type = outer_builder->build();

  assert(outer_type->is_consistent());
  assert(inner_type->is_consistent());


  // POPULATE DATA =================================================================================
  DynamicData * outer_data = data_factory->create_data(outer_type);
  DynamicData * outer_seq_member = outer_data->loan_value(0);

  outer_data->return_loaned_value(outer_seq_member);
  DynamicDataHelper::print(outer_data);


  // PUBSUB ========================================================================================
  TypeSupport outer_ts(new eprosima::fastrtps::types::DynamicPubSubType(outer_type));

  DomainParticipantQos pqos;
  pqos.name("Participant_pub");
  eprosima::fastdds::dds::DomainParticipant * mp_participant =
    DomainParticipantFactory::get_instance()->create_participant(0, pqos);

  if (mp_participant == nullptr) {
    return false;
  }

  outer_ts.get()->auto_fill_type_information(false);
  outer_ts.get()->auto_fill_type_object(true);

  std::cout << "[OK BEFORE REGISTERING TYPE]" << std::endl;
  outer_ts.register_type(mp_participant);
  std::cout << "SEGFAULT ^^^^^^^^^^^^^^^^^^^^^" << std::endl;

  return 0;
}
