/*
 * Copyright (c) 2013 taggit workers, All rights reserved.
 * Terms for redistribution and use can be found in LICENCE.
 */

/**
 * @file file-spec.cpp
 * @brief API for File specific routines and abstraction
 */

#ifndef INC_FILE_SPEC_H
#define INC_FILE_SPEC_H

#include <map>
#include <string>

#include "taggit.h"

enum file_type get_ext_type(std::string);
bool is_multitag_type(enum file_type);
bool taggit_open(struct taggit_file &);

std::string get_tag_types(const struct taggit_file &);
TagLib::PropertyMap get_tags_for_file(const struct taggit_file &);
bool tag_impl_allowed_for_file_type(enum file_type, enum tag_impl);
void tag_multitag(const struct taggit_file &);

extern std::map< enum file_type, std::vector< enum tag_impl > > filetag_map;

#endif /* INC_FILE_SPEC_H */