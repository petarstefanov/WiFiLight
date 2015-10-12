
#include "eagle_soc.h"
#include "endpoints.h"
#include "microcoap.h"
#include "Arduino.h"

typedef signed char 	int8_t;

typedef unsigned char 	uint8_t;


//typedef unsigned int 	uint16_t;


static char light = '0';
static const uint8_t BULB = 4;
const uint16_t rsplen = 100;
static char rsp[100] = "";
void build_rsp(void);


void endpoint_setup(void)
{
	pinMode(BUILTIN_LED, OUTPUT);
	pinMode(BULB, OUTPUT);
	build_rsp();
}


static const coap_endpoint_path_t path_well_known_core = { 2, { ".well-known", "core" } };
static int handle_get_well_known_core(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
	return coap_make_response(scratch, outpkt, (const uint8_t *)rsp, strlen(rsp), id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_APPLICATION_LINKFORMAT);
}

static const coap_endpoint_path_t path_light1 = { 1, { "builtinled" } };

static int handle_get_light(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
	return coap_make_response(scratch, outpkt, (const uint8_t *)&light, 1, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_put_light(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
	if (inpkt->payload.len == 0)
		return coap_make_response(scratch, outpkt, NULL, 0, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_BAD_REQUEST, COAP_CONTENTTYPE_TEXT_PLAIN);
	if (inpkt->payload.p[0] == '1')
	{
		light = '1';
		digitalWrite(BUILTIN_LED, LOW);
		digitalWrite(BULB, LOW);
		return coap_make_response(scratch, outpkt, (const uint8_t *)&light, 1, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CHANGED, COAP_CONTENTTYPE_TEXT_PLAIN);
	}
	else
	{
		light = '0';
		digitalWrite(BUILTIN_LED, HIGH);
		digitalWrite(BULB, HIGH);

		return coap_make_response(scratch, outpkt, (const uint8_t *)&light, 1, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CHANGED, COAP_CONTENTTYPE_TEXT_PLAIN);
	}
}

static int handle_get_light1(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
	return coap_make_response(scratch, outpkt, (const uint8_t *)&light, 1, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CONTENT, COAP_CONTENTTYPE_TEXT_PLAIN);
}

static int handle_put_light1(coap_rw_buffer_t *scratch, const coap_packet_t *inpkt, coap_packet_t *outpkt, uint8_t id_hi, uint8_t id_lo)
{
	if (inpkt->payload.len == 0)
		return coap_make_response(scratch, outpkt, NULL, 0, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_BAD_REQUEST, COAP_CONTENTTYPE_TEXT_PLAIN);
	if (inpkt->payload.p[0] == '1')
	{
		light = '1';
		gpio_output_set(BIT2, 0, BIT2, 0);
		return coap_make_response(scratch, outpkt, (const uint8_t *)&light, 1, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CHANGED, COAP_CONTENTTYPE_TEXT_PLAIN);
	}
	else
	{
		light = '0';
		gpio_output_set(0, BIT2, BIT2, 0);
		return coap_make_response(scratch, outpkt, (const uint8_t *)&light, 1, id_hi, id_lo, &inpkt->tok, COAP_RSPCODE_CHANGED, COAP_CONTENTTYPE_TEXT_PLAIN);
	}
}

const coap_endpoint_t endpoints[] =
{
	{ COAP_METHOD_GET, handle_get_well_known_core, &path_well_known_core, "ct=40" },
	{ COAP_METHOD_GET, handle_get_light, &path_light1, "ct=0" },
	{ COAP_METHOD_PUT, handle_put_light, &path_light1, NULL },
	{ (coap_method_t)0, NULL, NULL, NULL }
};

void build_rsp(void)
{

	uint16_t len = rsplen;
	const coap_endpoint_t *ep = endpoints;
	int i;

	len--; // Null-terminated string

	while (NULL != ep->handler)
	{
		if (NULL == ep->core_attr) {
			ep++;
			continue;
		}

		if (0 < strlen(rsp)) {
			strncat(rsp, ",", len);
			len--;
		}

		strncat(rsp, "<", len);
		len--;

		for (i = 0; i < ep->path->count; i++) {
			strncat(rsp, "/", len);
			len--;

			strncat(rsp, ep->path->elems[i], len);
			len -= strlen(ep->path->elems[i]);
		}

		strncat(rsp, ">;", len);
		len -= 2;

		strncat(rsp, ep->core_attr, len);
		len -= strlen(ep->core_attr);

		ep++;
	}
}

