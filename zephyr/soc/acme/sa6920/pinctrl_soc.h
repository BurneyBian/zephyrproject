

#include <zephyr/devicetree.h>
#include <zephyr/types.h>
#include <zephyr/dt-bindings/pinctrl/sa6920_pinctrl.h>


#ifdef __cplusplus
extern "C" {
#endif

/** Type for acme pin. */
typedef struct pinctrl_soc_pin {
	/** Pinmux settings (port, pin and function). */
	uint32_t pinmux;
	/** Pin configuration (bias, drive and slew rate). */
	uint32_t pincfg;
} pinctrl_soc_pin_t;

/**
 * @brief Definitions used to initialize fields in #pinctrl_pin_t
 */
#define SA6920_NO_PULL     0x2
#define SA6920_PULL_UP     0x3
#define SA6920_PULL_DOWN   0x1
#define SA6920_PUSH_PULL   0x0
#define SA6920_OPEN_DRAIN  0x1


#define Z_PINCTRL_SA6920_PINMUX_INIT(node_id) DT_PROP(node_id, pinmux)

#define Z_PINCTRL_SA6920_PINCFG_INIT(node_id)				       \
	(((SA6920_NO_PULL * DT_PROP(node_id, bias_disable)) << SA6920_PUPDR_SHIFT) | \
	 ((SA6920_PULL_UP * DT_PROP(node_id, bias_pull_up)) << SA6920_PUPDR_SHIFT) | \
	 ((SA6920_PULL_DOWN * DT_PROP(node_id, bias_pull_down)) << SA6920_PUPDR_SHIFT) | \
	 ((SA6920_PUSH_PULL * DT_PROP(node_id, drive_push_pull)) << SA6920_OTYPER_SHIFT) | \
	 ((SA6920_OPEN_DRAIN * DT_PROP(node_id, drive_open_drain)) << SA6920_OTYPER_SHIFT) | \
	 (DT_ENUM_IDX(node_id, drive_strength) << SA6920_OSPEEDR_SHIFT))
/**
 * @brief Utility macro to initialize each pin.
 *
 * @param node_id Node identifier.
 * @param state_prop State property name.
 * @param idx State property entry index.
 */
#define Z_PINCTRL_STATE_PIN_INIT(node_id, state_prop, idx)		       \
	{ .pinmux = Z_PINCTRL_SA6920_PINMUX_INIT(			       \
		DT_PROP_BY_IDX(node_id, state_prop, idx)),		       \
	  .pincfg = Z_PINCTRL_SA6920_PINCFG_INIT(			       \
		DT_PROP_BY_IDX(node_id, state_prop, idx)) },

/**
 * @brief Utility macro to initialize state pins contained in a given property.
 *
 * @param node_id Node identifier.
 * @param prop Property name describing state pins.
 */
#define Z_PINCTRL_STATE_PINS_INIT(node_id, prop)			       \
	{DT_FOREACH_PROP_ELEM(node_id, prop, Z_PINCTRL_STATE_PIN_INIT)}

#ifdef __cplusplus
}
#endif