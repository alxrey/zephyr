/*
 * Copyright (c) 2026
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/net/sdp_parser.h>
#include <zephyr/sys/errno.h>

#include <string.h>

#define LOG_LEVEL CONFIG_NET_SDP_LOG_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(net_sdp);

struct sdp_token {
	const char *ptr;
	size_t len;
};

static int tokenise_n(const char *buf, size_t len, struct sdp_token *tokens, size_t token_count)
{
	size_t token_idx = 0U;
	size_t cursor = 0U;

	if ((tokens == NULL) || (token_count == 0U)) {
		return -EINVAL;
	}

	while ((cursor < len) && (token_idx < token_count)) {
		size_t start;
		size_t end;

		while ((cursor < len) && (buf[cursor] == ' ')) {
			cursor++;
		}

		if (cursor == len) {
			break;
		}

		start = cursor;
		while ((cursor < len) && (buf[cursor] != ' ')) {
			cursor++;
		}
		end = cursor;

		tokens[token_idx].ptr = &buf[start];
		tokens[token_idx].len = end - start;
		token_idx++;
	}

	if (token_idx != token_count) {
		return -EINVAL;
	}

	while (cursor < len) {
		if (buf[cursor] != ' ') {
			return -EINVAL;
		}
		cursor++;
	}

	return 0;
}

static bool has_newline(const char *str)
{
	size_t i;
	size_t len;

	if (str == NULL) {
		return false;
	}

	len = strlen(str);
	for (i = 0U; i < len; i++) {
		if ((str[i] == '\r') || (str[i] == '\n')) {
			return true;
		}
	}

	return false;
}

bool sdp_line_type_is_valid(char type)
{
	switch (type) {
	case SDP_TYPE_VERSION:
	case SDP_TYPE_ORIGIN:
	case SDP_TYPE_SESSION_NAME:
	case SDP_TYPE_INFO:
	case SDP_TYPE_URI:
	case SDP_TYPE_EMAIL:
	case SDP_TYPE_PHONE:
	case SDP_TYPE_CONNECTION:
	case SDP_TYPE_BANDWIDTH:
	case SDP_TYPE_TIME:
	case SDP_TYPE_REPEAT:
	case SDP_TYPE_TIME_ZONE:
	case SDP_TYPE_ENCRYPTION:
	case SDP_TYPE_ATTRIBUTE:
	case SDP_TYPE_MEDIA:
		return true;
	default:
		return false;
	}
}

void sdp_parser_init(struct sdp_parser *parser, const char *data, size_t len)
{
	if (parser == NULL) {
		return;
	}

	parser->buf = data;
	parser->len = len;
	parser->pos = 0U;
	parser->line_no = 0U;
}

int sdp_parser_next(struct sdp_parser *parser, struct sdp_line *line)
{
	size_t start;
	size_t end;
	char type;

	if ((parser == NULL) || (line == NULL) || (parser->buf == NULL && parser->len != 0U)) {
		return -EINVAL;
	}

	while (parser->pos < parser->len) {
		start = parser->pos;
		end = start;

		while ((end < parser->len) && (parser->buf[end] != '\n')) {
			end++;
		}

		parser->pos = (end < parser->len) ? (end + 1U) : end;
		parser->line_no++;

		if ((end > start) && (parser->buf[end - 1U] == '\r')) {
			end--;
		}

		if (end == start) {
			continue;
		}

		if (((end - start) < 2U) || (parser->buf[start + 1U] != '=')) {
			LOG_DBG("Malformed SDP line %u", parser->line_no);
			return -EINVAL;
		}

		type = parser->buf[start];
		if (!sdp_line_type_is_valid(type)) {
			LOG_DBG("Unsupported SDP type '%c' at line %u", type, parser->line_no);
			return -EINVAL;
		}

		if (type == SDP_TYPE_ENCRYPTION) {
			LOG_DBG("Discarding obsolete SDP k= line at line %u", parser->line_no);
			continue;
		}

		line->type = type;
		line->value = &parser->buf[start + 2U];
		line->value_len = end - start - 2U;
		line->line_no = parser->line_no;
		return 0;
	}

	return -ENOENT;
}

int sdp_parse_attribute(const struct sdp_line *line, struct sdp_attribute *attr)
{
	const char *sep;

	if ((line == NULL) || (attr == NULL) || (line->type != SDP_TYPE_ATTRIBUTE)) {
		return -EINVAL;
	}

	sep = memchr(line->value, ':', line->value_len);
	if (sep == NULL) {
		attr->name = line->value;
		attr->name_len = line->value_len;
		attr->value = NULL;
		attr->value_len = 0U;
		attr->has_value = false;
	} else {
		attr->name = line->value;
		attr->name_len = (size_t)(sep - line->value);
		attr->value = sep + 1;
		attr->value_len = line->value_len - attr->name_len - 1U;
		attr->has_value = true;
	}

	if (attr->name_len == 0U) {
		return -EINVAL;
	}

	return 0;
}

int sdp_parse_origin(const struct sdp_line *line, struct sdp_origin *origin)
{
	struct sdp_token tok[6];
	int ret;

	if ((line == NULL) || (origin == NULL) || (line->type != SDP_TYPE_ORIGIN)) {
		return -EINVAL;
	}

	ret = tokenise_n(line->value, line->value_len, tok, ARRAY_SIZE(tok));
	if (ret < 0) {
		return ret;
	}

	origin->username = tok[0].ptr;
	origin->username_len = tok[0].len;
	origin->sess_id = tok[1].ptr;
	origin->sess_id_len = tok[1].len;
	origin->sess_version = tok[2].ptr;
	origin->sess_version_len = tok[2].len;
	origin->nettype = tok[3].ptr;
	origin->nettype_len = tok[3].len;
	origin->addrtype = tok[4].ptr;
	origin->addrtype_len = tok[4].len;
	origin->unicast_address = tok[5].ptr;
	origin->unicast_address_len = tok[5].len;

	return 0;
}

int sdp_parse_connection(const struct sdp_line *line, struct sdp_connection *conn)
{
	struct sdp_token tok[3];
	int ret;

	if ((line == NULL) || (conn == NULL) || (line->type != SDP_TYPE_CONNECTION)) {
		return -EINVAL;
	}

	ret = tokenise_n(line->value, line->value_len, tok, ARRAY_SIZE(tok));
	if (ret < 0) {
		return ret;
	}

	conn->nettype = tok[0].ptr;
	conn->nettype_len = tok[0].len;
	conn->addrtype = tok[1].ptr;
	conn->addrtype_len = tok[1].len;
	conn->connection_address = tok[2].ptr;
	conn->connection_address_len = tok[2].len;

	return 0;
}

void sdp_writer_init(struct sdp_writer *writer, char *buf, size_t len)
{
	if (writer == NULL) {
		return;
	}

	writer->buf = buf;
	writer->len = len;
	writer->pos = 0U;
}

int sdp_writer_add_line(struct sdp_writer *writer, char type, const char *value)
{
	size_t value_len;
	size_t needed;

	if ((writer == NULL) || (value == NULL) || !sdp_line_type_is_valid(type) ||
	    (writer->buf == NULL && writer->len != 0U)) {
		return -EINVAL;
	}

	if (type == SDP_TYPE_ENCRYPTION) {
		return -ENOTSUP;
	}

	if (has_newline(value)) {
		return -EINVAL;
	}

	value_len = strlen(value);
	needed = 2U + value_len + 2U;
	if ((writer->len - writer->pos) < needed) {
		return -EMSGSIZE;
	}

	writer->buf[writer->pos++] = type;
	writer->buf[writer->pos++] = '=';
	memcpy(&writer->buf[writer->pos], value, value_len);
	writer->pos += value_len;
	writer->buf[writer->pos++] = '\r';
	writer->buf[writer->pos++] = '\n';

	return 0;
}

int sdp_writer_add_attribute(struct sdp_writer *writer, const char *name, const char *value)
{
	size_t name_len;
	size_t value_len = 0U;
	size_t needed;

	if ((writer == NULL) || (name == NULL) || (writer->buf == NULL && writer->len != 0U)) {
		return -EINVAL;
	}

	if (has_newline(name) || has_newline(value)) {
		return -EINVAL;
	}

	name_len = strlen(name);
	if (name_len == 0U) {
		return -EINVAL;
	}

	if (value != NULL) {
		value_len = strlen(value);
	}

	needed = 2U + name_len + 2U;
	if (value != NULL) {
		needed += 1U + value_len;
	}

	if ((writer->len - writer->pos) < needed) {
		return -EMSGSIZE;
	}

	writer->buf[writer->pos++] = SDP_TYPE_ATTRIBUTE;
	writer->buf[writer->pos++] = '=';
	memcpy(&writer->buf[writer->pos], name, name_len);
	writer->pos += name_len;
	if (value != NULL) {
		writer->buf[writer->pos++] = ':';
		memcpy(&writer->buf[writer->pos], value, value_len);
		writer->pos += value_len;
	}
	writer->buf[writer->pos++] = '\r';
	writer->buf[writer->pos++] = '\n';

	return 0;
}

int sdp_writer_terminate(struct sdp_writer *writer)
{
	if ((writer == NULL) || (writer->buf == NULL && writer->len != 0U)) {
		return -EINVAL;
	}

	if ((writer->len - writer->pos) < 1U) {
		return -EMSGSIZE;
	}

	writer->buf[writer->pos] = '\0';
	return 0;
}

size_t sdp_writer_len(const struct sdp_writer *writer)
{
	if (writer == NULL) {
		return 0U;
	}

	return writer->pos;
}
