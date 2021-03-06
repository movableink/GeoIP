/*
 * GeoIP C library binding for nodejs
 *
 * Licensed under the GNU LGPL 2.1 license
 */

#include "city.h"
#include "global.h"

using namespace native;

City::City() : db(NULL) {};

City::~City() { if (db) {
  GeoIP_delete(db);
}
};

Nan::Persistent<v8::Function> City::constructor;

void City::Init(v8::Local<v8::Object> exports) {
  v8::Local<v8::Context> context = exports->CreationContext();
  Nan::HandleScope scope;

  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("City").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  tpl->PrototypeTemplate()->Set(Nan::New("lookupSync").ToLocalChecked(),
                                Nan::New<v8::FunctionTemplate>(lookupSync));

  constructor.Reset(tpl->GetFunction(context).ToLocalChecked());
  Nan::Set(exports, Nan::New("City").ToLocalChecked(), tpl->GetFunction(context).ToLocalChecked());
}

NAN_METHOD(City::New) {
  Nan::HandleScope scope;

  City *c = new City();

  Nan::Utf8String utf8_value(info[0]);
  const char * file_cstr = *utf8_value;
  bool cache_on = info[1]->ToBoolean(Isolate::GetCurrent())->Value();

  c->db = GeoIP_open(file_cstr, cache_on ? GEOIP_MEMORY_CACHE : GEOIP_STANDARD);

  if (c->db) {
    c->db_edition = GeoIP_database_edition(c->db);
    if (c->db_edition == GEOIP_CITY_EDITION_REV0 ||
        c->db_edition == GEOIP_CITY_EDITION_REV1) {
      c->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    } else {
      GeoIP_delete(c->db);  // free()'s the reference & closes its fd
      return Nan::ThrowError("Error: Not valid city database");
    }
  } else {
    return Nan::ThrowError("Error: Cannot open database");
  }
}

NAN_METHOD(City::lookupSync) {
  Nan::HandleScope scope;

  City *c = ObjectWrap::Unwrap<City>(info.This());

  Local<Object> data = Nan::New<Object>();
  //static Nan::Utf8String *host_cstr = new Nan::Utf8String(info[0]);
  Nan::Utf8String host_cstr(info[0]);
  //printf("\nHost CStr is %s.\n", **host_cstr);

  //uint32_t ipnum = _GeoIP_lookupaddress(**host_cstr);
  uint32_t ipnum = _GeoIP_lookupaddress(*host_cstr);
  //printf("Ipnum is %d.", ipnum);

  if (ipnum == 0) {
    info.GetReturnValue().SetNull();
  }

  GeoIPRecord *record = GeoIP_record_by_ipnum(c->db, ipnum);

  if (!record) {
    info.GetReturnValue().SetNull();
    return;
  }

  if (record->country_code) {
    Nan::Set(data, Nan::New<String>("country_code").ToLocalChecked(),
      Nan::New<String>(record->country_code).ToLocalChecked());
  }

  if (record->country_code3) {
    Nan::Set(data, Nan::New<String>("country_code3").ToLocalChecked(),
      Nan::New<String>(record->country_code3).ToLocalChecked());
  }

  if (record->country_name) {
    Nan::Set(data, Nan::New<String>("country_name").ToLocalChecked(),
      Nan::New<String>(record->country_name).ToLocalChecked());
  }

  if (record->region) {
    Nan::Set(data, Nan::New<String>("region").ToLocalChecked(),
      Nan::New<String>(record->region).ToLocalChecked());
  }

  if (record->city) {
    char *name = _GeoIP_iso_8859_1__utf8(record->city);

    if (name) {
      Nan::Set(data, Nan::New<String>("city").ToLocalChecked(),
        Nan::New<String>(name).ToLocalChecked());
    }

    free(name);
  }

  if (record->postal_code) {
    Nan::Set(data, Nan::New<String>("postal_code").ToLocalChecked(),
      Nan::New<String>(record->postal_code).ToLocalChecked());
  }

  if (record->latitude >= -90 && record->latitude <= 90) {
    Nan::Set(data, Nan::New<String>("latitude").ToLocalChecked(),
      Nan::New<Number>(record->latitude));;
  }

  if (record->longitude >= -180 && record->longitude <= 180) {
    Nan::Set(data, Nan::New<String>("longitude").ToLocalChecked(),
      Nan::New<Number>(record->longitude));
  }

  if (record->metro_code) {
    Nan::Set(data, Nan::New<String>("metro_code").ToLocalChecked(),
      Nan::New<Number>(record->metro_code));;
  }

  if (record->dma_code) {
    Nan::Set(data, Nan::New<String>("dma_code").ToLocalChecked(),
      Nan::New<Number>(record->dma_code));
  }

  if (record->area_code) {
    Nan::Set(data, Nan::New<String>("area_code").ToLocalChecked(),
      Nan::New<Number>(record->area_code));
  }

  if (record->continent_code) {
    Nan::Set(data, Nan::New<String>("continent_code").ToLocalChecked(),
      Nan::New<String>(record->continent_code).ToLocalChecked());
  }

  if (record->country_code && record->region) {
    const char *time_zone = GeoIP_time_zone_by_country_and_region(record->country_code, record->region);

    if(time_zone) {
      Nan::Set(data, Nan::New<String>("time_zone").ToLocalChecked(),
        Nan::New<String>(time_zone).ToLocalChecked());
    }
  }

  GeoIPRecord_delete(record);
  info.GetReturnValue().Set(data);
}
