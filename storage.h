#pragma once

#include <string>

namespace raftkv {

using namespace std;

class Storage {
   public:
    virtual ~Storage();
    virtual bool Get(const string& key, string* value) = 0;
    virtual void Put(const string& key,const string& value) = 0;
    virtual void Delete(const string& key) = 0;
};

}  // namespace raftkv
