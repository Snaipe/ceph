// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab ft=cpp

#ifndef CEPH_RGW_MULTI_H
#define CEPH_RGW_MULTI_H

#include <map>
#include "rgw_xml.h"
#include "rgw_obj_manifest.h"
#include "rgw_compression_types.h"
#include "common/dout.h"

namespace rgw { namespace sal {
  class Store;
} }

#define MULTIPART_UPLOAD_ID_PREFIX_LEGACY "2/"
#define MULTIPART_UPLOAD_ID_PREFIX "2~" // must contain a unique char that may not come up in gen_rand_alpha()

class RGWMPObj;

struct RGWUploadPartInfo {
  uint32_t num;
  uint64_t size;
  uint64_t accounted_size{0};
  string etag;
  ceph::real_time modified;
  RGWObjManifest manifest;
  RGWCompressionInfo cs_info;

  RGWUploadPartInfo() : num(0), size(0) {}

  void encode(bufferlist& bl) const {
    ENCODE_START(4, 2, bl);
    encode(num, bl);
    encode(size, bl);
    encode(etag, bl);
    encode(modified, bl);
    encode(manifest, bl);
    encode(cs_info, bl);
    encode(accounted_size, bl);
    ENCODE_FINISH(bl);
  }
  void decode(bufferlist::const_iterator& bl) {
    DECODE_START_LEGACY_COMPAT_LEN(4, 2, 2, bl);
    decode(num, bl);
    decode(size, bl);
    decode(etag, bl);
    decode(modified, bl);
    if (struct_v >= 3)
      decode(manifest, bl);
    if (struct_v >= 4) {
      decode(cs_info, bl);
      decode(accounted_size, bl);
    } else {
      accounted_size = size;
    }
    DECODE_FINISH(bl);
  }
  void dump(Formatter *f) const;
  static void generate_test_instances(list<RGWUploadPartInfo*>& o);
};
WRITE_CLASS_ENCODER(RGWUploadPartInfo)

class RGWMultiCompleteUpload : public XMLObj
{
public:
  RGWMultiCompleteUpload() {}
  ~RGWMultiCompleteUpload() override {}
  bool xml_end(const char *el) override;

  std::map<int, string> parts;
};

class RGWMultiPart : public XMLObj
{
  string etag;
  int num;
public:
  RGWMultiPart() : num(0) {}
  ~RGWMultiPart() override {}
  bool xml_end(const char *el) override;

  string& get_etag() { return etag; }
  int get_num() { return num; }
};

class RGWMultiPartNumber : public XMLObj
{
public:
  RGWMultiPartNumber() {}
  ~RGWMultiPartNumber() override {}
};

class RGWMultiETag : public XMLObj
{
public:
  RGWMultiETag() {}
  ~RGWMultiETag() override {}
};

class RGWMultiXMLParser : public RGWXMLParser
{
  XMLObj *alloc_obj(const char *el) override;
public:
  RGWMultiXMLParser() {}
  virtual ~RGWMultiXMLParser() override;
};

extern bool is_v2_upload_id(const string& upload_id);

#endif
