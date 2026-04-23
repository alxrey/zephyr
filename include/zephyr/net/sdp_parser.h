/*
 * Copyright (c) 2026
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Session Description Protocol (SDP) low-level parser and writer
 */

#ifndef ZEPHYR_INCLUDE_NET_SDP_PARSER_H_
#define ZEPHYR_INCLUDE_NET_SDP_PARSER_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Protocol Version ("v=") */
#define SDP_TYPE_VERSION      'v'
/** Origin ("o=") */
#define SDP_TYPE_ORIGIN       'o'
/** Session Name ("s=") */
#define SDP_TYPE_SESSION_NAME 's'
/** Session Information ("i=") */
#define SDP_TYPE_INFO         'i'
/** URI ("u=") */
#define SDP_TYPE_URI          'u'
/** Email Address ("e=") */
#define SDP_TYPE_EMAIL        'e'
/** Phone Number ("p=") */
#define SDP_TYPE_PHONE        'p'
/** Connection Information ("c=") */
#define SDP_TYPE_CONNECTION   'c'
/** Bandwidth Information ("b=") */
#define SDP_TYPE_BANDWIDTH    'b'
/** Time Active ("t=") */
#define SDP_TYPE_TIME         't'
/** Repeat Times ("r=") */
#define SDP_TYPE_REPEAT       'r'
/** Time Zone Adjustments ("z=") */
#define SDP_TYPE_TIME_ZONE    'z'
/** Encryption Key ("k="), obsolete in RFC 8866 */
#define SDP_TYPE_ENCRYPTION   'k'
/** Attributes ("a=") */
#define SDP_TYPE_ATTRIBUTE    'a'
/** Media Descriptions ("m=") */
#define SDP_TYPE_MEDIA        'm'

/** Parsed SDP line (`<type>=<value>`). */
struct sdp_line {
	char type;
	const char *value;
	size_t value_len;
	uint16_t line_no;
};

/** Stateful SDP parser context. */
struct sdp_parser {
	const char *buf;
	size_t len;
	size_t pos;
	uint16_t line_no;
};

/** Parsed value of an `a=` line. */
struct sdp_attribute {
	const char *name;
	size_t name_len;
	const char *value;
	size_t value_len;
	bool has_value;
};

/** Parsed value of an `o=` line. */
struct sdp_origin {
	const char *username;
	size_t username_len;
	const char *sess_id;
	size_t sess_id_len;
	const char *sess_version;
	size_t sess_version_len;
	const char *nettype;
	size_t nettype_len;
	const char *addrtype;
	size_t addrtype_len;
	const char *unicast_address;
	size_t unicast_address_len;
};

/** Parsed value of a `c=` line. */
struct sdp_connection {
	const char *nettype;
	size_t nettype_len;
	const char *addrtype;
	size_t addrtype_len;
	const char *connection_address;
	size_t connection_address_len;
};

/** SDP writer context. */
struct sdp_writer {
	char *buf;
	size_t len;
	size_t pos;
};

bool sdp_line_type_is_valid(char type);
void sdp_parser_init(struct sdp_parser *parser, const char *data, size_t len);
int sdp_parser_next(struct sdp_parser *parser, struct sdp_line *line);
int sdp_parse_attribute(const struct sdp_line *line, struct sdp_attribute *attr);
int sdp_parse_origin(const struct sdp_line *line, struct sdp_origin *origin);
int sdp_parse_connection(const struct sdp_line *line, struct sdp_connection *conn);
void sdp_writer_init(struct sdp_writer *writer, char *buf, size_t len);
int sdp_writer_add_line(struct sdp_writer *writer, char type, const char *value);
int sdp_writer_add_attribute(struct sdp_writer *writer, const char *name, const char *value);
int sdp_writer_terminate(struct sdp_writer *writer);
size_t sdp_writer_len(const struct sdp_writer *writer);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_NET_SDP_PARSER_H_ */
