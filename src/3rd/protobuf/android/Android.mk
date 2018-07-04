
LOCAL_PATH:= $(call my-dir)
TOP_PATH:= $(LOCAL_PATH)/../
SRC_PATH:= $(TOP_PATH)/

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_PROTOBUF_FILES := \
			$(SRC_PATH)/src/google/protobuf/any.cc \
			$(SRC_PATH)/src/google/protobuf/any.pb.cc \
			$(SRC_PATH)/src/google/protobuf/api.pb.cc  \
			$(SRC_PATH)/src/google/protobuf/compiler/importer.cc \
			$(SRC_PATH)/src/google/protobuf/compiler/parser.cc \
			$(SRC_PATH)/src/google/protobuf/descriptor.cc \
			$(SRC_PATH)/src/google/protobuf/descriptor.pb.cc \
			$(SRC_PATH)/src/google/protobuf/descriptor_database.cc \
			$(SRC_PATH)/src/google/protobuf/duration.pb.cc \
			$(SRC_PATH)/src/google/protobuf/dynamic_message.cc \
			$(SRC_PATH)/src/google/protobuf/empty.pb.cc \
			$(SRC_PATH)/src/google/protobuf/extension_set_heavy.cc \
			$(SRC_PATH)/src/google/protobuf/field_mask.pb.cc \
			$(SRC_PATH)/src/google/protobuf/generated_message_reflection.cc \
			$(SRC_PATH)/src/google/protobuf/io/gzip_stream.cc \
			$(SRC_PATH)/src/google/protobuf/io/printer.cc \
			$(SRC_PATH)/src/google/protobuf/io/strtod.cc \
			$(SRC_PATH)/src/google/protobuf/io/tokenizer.cc \
			$(SRC_PATH)/src/google/protobuf/io/zero_copy_stream_impl.cc \
			$(SRC_PATH)/src/google/protobuf/map_field.cc \
			$(SRC_PATH)/src/google/protobuf/message.cc \
			$(SRC_PATH)/src/google/protobuf/reflection_ops.cc \
			$(SRC_PATH)/src/google/protobuf/service.cc \
			$(SRC_PATH)/src/google/protobuf/source_context.pb.cc \
			$(SRC_PATH)/src/google/protobuf/struct.pb.cc \
			$(SRC_PATH)/src/google/protobuf/stubs/mathlimits.cc \
			$(SRC_PATH)/src/google/protobuf/stubs/substitute.cc \
			$(SRC_PATH)/src/google/protobuf/text_format.cc \
			$(SRC_PATH)/src/google/protobuf/timestamp.pb.cc \
			$(SRC_PATH)/src/google/protobuf/type.pb.cc \
			$(SRC_PATH)/src/google/protobuf/unknown_field_set.cc \
			$(SRC_PATH)/src/google/protobuf/util/field_comparator.cc \
			$(SRC_PATH)/src/google/protobuf/util/field_mask_util.cc \
			$(SRC_PATH)/src/google/protobuf/util/internal/datapiece.cc \
			$(SRC_PATH)/src/google/protobuf/util/internal/default_value_objectwriter.cc \
			$(SRC_PATH)/src/google/protobuf/util/internal/error_listener.cc \
			$(SRC_PATH)/src/google/protobuf/util/internal/field_mask_utility.cc \
			$(SRC_PATH)/src/google/protobuf/util/internal/json_escaping.cc \
			$(SRC_PATH)/src/google/protobuf/util/internal/json_objectwriter.cc \
			$(SRC_PATH)/src/google/protobuf/util/internal/json_stream_parser.cc \
			$(SRC_PATH)/src/google/protobuf/util/internal/object_writer.cc \
			$(SRC_PATH)/src/google/protobuf/util/internal/proto_writer.cc \
			$(SRC_PATH)/src/google/protobuf/util/internal/protostream_objectsource.cc \
			$(SRC_PATH)/src/google/protobuf/util/internal/protostream_objectwriter.cc \
			$(SRC_PATH)/src/google/protobuf/util/internal/type_info.cc \
			$(SRC_PATH)/src/google/protobuf/util/internal/type_info_test_helper.cc \
			$(SRC_PATH)/src/google/protobuf/util/internal/utility.cc \
			$(SRC_PATH)/src/google/protobuf/util/json_util.cc \
			$(SRC_PATH)/src/google/protobuf/util/message_differencer.cc \
			$(SRC_PATH)/src/google/protobuf/util/time_util.cc \
			$(SRC_PATH)/src/google/protobuf/util/type_resolver_util.cc \
			$(SRC_PATH)/src/google/protobuf/wire_format.cc \
			$(SRC_PATH)/src/google/protobuf/wrappers.pb.cc \
			$(SRC_PATH)src/google/protobuf/arena.cc \
			$(SRC_PATH)src/google/protobuf/arenastring.cc \
			$(SRC_PATH)src/google/protobuf/extension_set.cc \
			$(SRC_PATH)src/google/protobuf/generated_message_util.cc \
			$(SRC_PATH)src/google/protobuf/io/coded_stream.cc \
			$(SRC_PATH)src/google/protobuf/io/zero_copy_stream.cc \
			$(SRC_PATH)src/google/protobuf/io/zero_copy_stream_impl_lite.cc \
			$(SRC_PATH)src/google/protobuf/message_lite.cc \
			$(SRC_PATH)src/google/protobuf/repeated_field.cc \
			$(SRC_PATH)src/google/protobuf/stubs/atomicops_internals_x86_gcc.cc \
			$(SRC_PATH)src/google/protobuf/stubs/atomicops_internals_x86_msvc.cc \
			$(SRC_PATH)src/google/protobuf/stubs/bytestream.cc \
			$(SRC_PATH)src/google/protobuf/stubs/common.cc \
			$(SRC_PATH)src/google/protobuf/stubs/int128.cc \
			$(SRC_PATH)src/google/protobuf/stubs/once.cc \
			$(SRC_PATH)src/google/protobuf/stubs/status.cc \
			$(SRC_PATH)src/google/protobuf/stubs/statusor.cc \
			$(SRC_PATH)src/google/protobuf/stubs/stringpiece.cc \
			$(SRC_PATH)src/google/protobuf/stubs/stringprintf.cc \
			$(SRC_PATH)src/google/protobuf/stubs/structurally_valid.cc \
			$(SRC_PATH)src/google/protobuf/stubs/strutil.cc \
			$(SRC_PATH)src/google/protobuf/stubs/time.cc \
			$(SRC_PATH)src/google/protobuf/wire_format_lite.cc


LOCAL_SRC_FILES := \
	$(LOCAL_SRC_PROTOBUF_FILES) 
	

LOCAL_C_INCLUDES += \
	$(SRC_PATH)/src/ \
	$(TOP_PATH)/include

#LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog 

LOCAL_CPPFLAGS := -DHAMMER_TIME=1 \
		  -DHASHNAMESPACE=__gnu_cxx \
		  -DHASH_NAMESPACE=__gnu_cxx \
		  -DDISABLE_DYNAMIC_CAST \
		  -D_REENTRANT \
		  -D__ANDROID__ \
		  -DHAVE_PTHREAD=1 \
		  -DHAVE_ZLIB=1 \
		  -DNDEBUG \
		  

LOCAL_CFLAGS := -fexpensive-optimizations -fexceptions -pthread \
		-mfpu=neon -mfloat-abi=softfp -flax-vector-conversions -fPIC  -O2 -g -D__STDC_CONSTANT_MACROS -Wno-sign-compare -Wno-switch 

LOCAL_MODULE:= libprotobuf

include $(BUILD_STATIC_LIBRARY)
#include $(BUILD_SHARED_LIBRARY)

