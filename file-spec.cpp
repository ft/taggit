/*
 * Copyright (c) 2013 taggit workers, All rights reserved.
 * Terms for redistribution and use can be found in LICENCE.
 */

/**
 * @file file-spec.cpp
 * @brief File specific routines and abstraction
 */

#include <algorithm>
#include <map>
#include <string>

#include <apetag.h>
#include <id3v1tag.h>
#include <id3v2tag.h>
#include <mpegfile.h>
#include <oggflacfile.h>
#include <tfile.h>
#include <tpropertymap.h>
#include <vorbisfile.h>

#include "setup.h"
#include "taggit.h"

static std::map< enum tag_impl, std::string > tagimpl_map {
    { TAG_T_APETAG, "apetag" },
    { TAG_T_ID3V1,  "id3v1" },
    { TAG_T_ID3V2,  "id3v2" },
    { TAG_T_NONE,   "none" },
};

/**
 * Map of file types that support multiple tag-types.
 *
 * This is used by both the read-map and the write-map. It also defines default
 * values for those:
 *
 * - The read-map defaults to exactly this map.
 *
 * - The write-map defaults to the first entry in the tag-type vector.
 */
static std::map< enum file_type, std::vector< enum tag_impl > > filetag_map {
    { FILE_T_MP3, { TAG_T_ID3V2, TAG_T_APETAG, TAG_T_ID3V1 } }
};

static std::vector< enum tag_impl >
get_vector_from_map(enum file_type type,
                    std::map< enum file_type, std::vector< enum tag_impl > > m)
{
    auto rv = m.find(type);
    if (rv == m.end())
        return {};
    return rv->second;
}

static std::vector< enum tag_impl >
get_readmap_vector(enum file_type type)
{
    /* TODO: Use read_map once it's set up. */
    return get_vector_from_map(type, /*here*/ filetag_map);
}

static std::vector< enum tag_impl >
get_multitag_vector(enum file_type type)
{
    return get_vector_from_map(type, filetag_map);
}

static bool
mp3_has_tag_type(TagLib::MPEG::File *fh, enum tag_impl type)
{
    if (type == TAG_T_APETAG && fh->hasAPETag())
        return true;
    if (type == TAG_T_ID3V2 && fh->hasID3v2Tag())
        return true;
    if (type == TAG_T_ID3V1 && fh->hasID3v1Tag())
        return true;
    return false;
}

static bool
has_tag_type(const struct taggit_file &file, enum tag_impl type)
{
    switch (file.type) {
    case FILE_T_MP3:
        return
            mp3_has_tag_type(
                reinterpret_cast<TagLib::MPEG::File *>(file.fh),
                type);
        break;
    default:
        return false;
    }
}

bool
is_multitag_type(enum file_type type)
{
    if (filetag_map.find(type) == filetag_map.end())
        return false;
    return true;
}

static std::map < std::string, enum file_type >
file_ext_map = {
    { "flac", FILE_T_OGG_FLAC },
    { "flc",  FILE_T_OGG_FLAC },
    { "mp3",  FILE_T_MP3 },
    { "ogg",  FILE_T_OGG_VORBIS },
    { "oga",  FILE_T_OGG_VORBIS }
};

static std::map < std::string, enum file_type >
file_type_map = {
    { "ogg-flac",     FILE_T_OGG_FLAC },
    { "ogg-vorbis",   FILE_T_OGG_VORBIS },
    { "mp3",          FILE_T_MP3 }
};

std::string
get_file_type_reverse(enum file_type type)
{
    for (auto &iter : file_type_map)
        if (iter.second == type)
            return iter.first;

    return "unknown-filetype";
}

enum file_type
get_file_type(std::string type)
{
    auto rv = file_type_map.find(type);
    if (rv == file_type_map.end())
        return FILE_T_INVALID;
    return rv->second;
}

enum file_type
get_ext_type(std::string filename)
{
    int dotidx = filename.rfind(".");
    std::string ext = filename.substr(dotidx + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    auto rv = file_ext_map.find(ext);
    if (rv == file_ext_map.end())
        return FILE_T_INVALID;
    return rv->second;
}


static enum tag_impl
get_prefered_tag_impl(const struct taggit_file &file)
{
    auto types = get_readmap_vector(file.type);
    for (auto &iter : types)
        if (has_tag_type(file, iter))
            return iter;
    return TAG_T_NONE;
}

bool
taggit_open(struct taggit_file &file)
{
    switch (file.type) {
    case FILE_T_MP3:
        file.fh = new TagLib::MPEG::File(file.name);
        break;
    case FILE_T_OGG_FLAC:
        file.fh = new TagLib::Ogg::FLAC::File(file.name);
        break;
    case FILE_T_OGG_VORBIS:
        file.fh = new TagLib::Ogg::Vorbis::File(file.name);
        break;
    default:
        return false;
    }

    if (!file.fh->isValid()) {
        std::cerr << PROJECT ": Could not open file: `"
                  << file.name << "'" << std::endl;
        goto error;
    }
    if (!file.fh->tag()) {
        std::cerr << PROJECT ": No tags in file: `"
                  << file.name << "'" << std::endl;
        goto error;
    }
    if (!file.fh->tag()) {
        std::cerr << PROJECT ": Could not get audio properties for file: `"
                  << file.name << "'" << std::endl;
        goto error;
    }

    if (is_multitag_type(file.type)) {
        file.multi_tag = true;
        file.tagimpl = get_prefered_tag_impl(file);
    } else
        file.multi_tag = false;

    return true;
error:
    delete file.fh;
    return false;
}

std::string
tag_impl_to_string(enum tag_impl type)
{
    auto rv = tagimpl_map.find(type);
    if (rv == tagimpl_map.end())
        return "unknown-tag-implementation";
    return rv->second;
}

std::string
get_tag_types(const struct taggit_file &file)
{
    std::string rv("");
    bool first = true;
    auto types = get_multitag_vector(file.type);
    for (auto &iter : types) {
        if (has_tag_type(file, iter)) {
            if (!first)
                rv += ",";
            else
                first = false;
            rv += tag_impl_to_string(iter);
        }
    }
    return first ? "none" : rv;
}

TagLib::PropertyMap
get_tags_for_file(const struct taggit_file &file)
{
    TagLib::MPEG::File *mp3fh;

    switch (file.type) {
    case FILE_T_MP3:
        mp3fh = reinterpret_cast<TagLib::MPEG::File *>(file.fh);
        switch (file.tagimpl) {
        case TAG_T_ID3V2:
            return mp3fh->ID3v2Tag()->properties();
        case TAG_T_APETAG:
            return mp3fh->APETag()->properties();
        case TAG_T_ID3V1:
            return mp3fh->ID3v1Tag()->properties();
        default:
            break;
        }
        /* FALL-THROUGH */
    default:
        return file.fh->properties();
    }
}
