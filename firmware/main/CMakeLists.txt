idf_component_register(SRCS "blizzard_sound_manager.c" "blizzard_dronie_manager.c" "blizzard_helpers.c" "blizzard_artnet_manager.c" "blizzard_rdm_manager.c" "blizzard_battery_manager.c" "blizzard_debuggers.c" "blizzard_dmx_manager.c" "blizzard_generators.c" "blizzard_json_manager.c" "blizzard_network_manager.c" "blizzard_nvs_manager.c"  "blizzard_ota_manager.c"  "blizzard_protocol_manager.c"  "blizzard_recorder_manager.c"  "blizzard_sacn_manager.c"  "blizzard_show_manager.c"  "blizzard_simple_button_manager.c" "blizzard_simple_indication_manager.c" "blizzard_simple_ui_manager.c" "blizzard_timer_manager.c" "blizzard_ws2812_manager.c" "main.c"
                    INCLUDE_DIRS "include"
                    REQUIRES ulp nvs_flash app_update esp_common esp_netif wpa_supplicant)

set(ulp_app_name ulp_sleep)
set(ulp_s_sources ulp/blizzard_sleep.S)
set(ulp_exp_dep_srcs "blizzard_battery_manager.c")

ulp_embed_binary(${ulp_app_name} "${ulp_s_sources}" "${ulp_exp_dep_srcs}")
