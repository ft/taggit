/*
 * Copyright (c) 2009 taggit workers, All rights reserved.
 * Terms for redistribution and use can be found in LICENCE.
 */

/**
 * @file setup.h
 * @brief API for taggit's configurable setup
 */

#ifndef INC_SETUP_H
#define INC_SETUP_H

#include <cstdint>
#include <map>
#include <vector>

#include "value.h"

void add_tag(enum tag_id, Value);
void set_opt(uint32_t);
bool get_opt(uint32_t);

extern std::map< enum file_type, std::vector< enum tag_impl > > read_map;
extern std::map< enum file_type, std::vector< enum tag_impl > > write_map;
extern std::map< enum tag_id, Value > newtags;

#endif /* INC_SETUP_H */