/******************************************************************************
 *
 *  Copyright (C) 2003-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  Changes from Qualcomm Innovation Center are provided under the following license:
 *
 *  Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted (subject to the limitations in the
 *  disclaimer below) provided that the following conditions are met:
 *
 *  Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *
 *  Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following
 *  disclaimer in the documentation and/or other materials provided
 *  with the distribution.
 *
 *  Neither the name of Qualcomm Innovation Center, Inc. nor the names of its
 *  contributors may be used to endorse or promote products derived
 *  from this software without specific prior written permission.
 *
 *  NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 *  GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 *  HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 *  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 *  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 *  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 *
 ******************************************************************************/

/******************************************************************************
 *
 *  This is the main implementation file for the BTA device manager.
 *
 ******************************************************************************/

#include "bta_api.h"
#include "bta_dm_int.h"
#include "bta_sys.h"

/*****************************************************************************
 * Constants and types
 ****************************************************************************/

tBTA_DM_CB bta_dm_cb;
tBTA_DM_SEARCH_CB bta_dm_search_cb;
tBTA_DM_DI_CB bta_dm_di_cb;

#define BTA_DM_NUM_ACTIONS (BTA_DM_MAX_EVT & 0x00ff)

/* type for action functions */
typedef void (*tBTA_DM_ACTION)(tBTA_DM_MSG* p_data);

/* action function list */
const tBTA_DM_ACTION bta_dm_action[] = {

    /* device manager local device API events */
    bta_dm_enable,         /* 0  BTA_DM_API_ENABLE_EVT */
    bta_dm_disable,        /* 1  BTA_DM_API_DISABLE_EVT */
    bta_dm_set_dev_name,   /* 2  BTA_DM_API_SET_NAME_EVT */
    bta_dm_set_visibility, /* 3  BTA_DM_API_SET_VISIBILITY_EVT */
    bta_dm_acl_change,     /* 8  BTA_DM_ACL_CHANGE_EVT */
    bta_dm_add_device,     /* 9  BTA_DM_API_ADD_DEVICE_EVT */
    bta_dm_close_acl,      /* 10  BTA_DM_API_ADD_DEVICE_EVT */

    /* security API events */
    bta_dm_bond,        /* 11  BTA_DM_API_BOND_EVT */
    bta_dm_bond_cancel, /* 12  BTA_DM_API_BOND_CANCEL_EVT */
    bta_dm_pin_reply,   /* 13 BTA_DM_API_PIN_REPLY_EVT */

    /* power manger events */
    bta_dm_pm_btm_status, /* 16 BTA_DM_PM_BTM_STATUS_EVT */
    bta_dm_pm_timer,      /* 17 BTA_DM_PM_TIMER_EVT*/

    /* simple pairing events */
    bta_dm_confirm, /* 18 BTA_DM_API_CONFIRM_EVT */

    bta_dm_set_encryption, /* BTA_DM_API_SET_ENCRYPTION_EVT */

    /* out of band pairing events */
    bta_dm_loc_oob,        /* 20 BTA_DM_API_LOC_OOB_EVT */
    bta_dm_ci_io_req_act,  /* 21 BTA_DM_CI_IO_REQ_EVT */
    bta_dm_ci_rmt_oob_act, /* 22 BTA_DM_CI_RMT_OOB_EVT */

    bta_dm_add_blekey,        /*  BTA_DM_API_ADD_BLEKEY_EVT           */
    bta_dm_add_ble_device,    /*  BTA_DM_API_ADD_BLEDEVICE_EVT        */
    bta_dm_ble_passkey_reply, /*  BTA_DM_API_BLE_PASSKEY_REPLY_EVT    */
    bta_dm_ble_confirm_reply, /*  BTA_DM_API_BLE_CONFIRM_REPLY_EVT    */
    bta_dm_security_grant, bta_dm_ble_set_bg_conn_type,
    bta_dm_ble_set_conn_params,      /* BTA_DM_API_BLE_CONN_PARAM_EVT */
    bta_dm_ble_set_conn_scan_params, /* BTA_DM_API_BLE_CONN_SCAN_PARAM_EVT */
    bta_dm_ble_observe,
    bta_dm_ble_update_conn_params, /* BTA_DM_API_UPDATE_CONN_PARAM_EVT */
#if (BLE_PRIVACY_SPT == TRUE)
    bta_dm_ble_config_local_privacy, /* BTA_DM_API_LOCAL_PRIVACY_EVT */
#endif
    bta_dm_ble_set_data_length, /* BTA_DM_API_SET_DATA_LENGTH_EVT */
    bta_dm_ble_get_energy_info,    /* BTA_DM_API_BLE_ENERGY_INFO_EVT */

    bta_dm_enable_test_mode,  /*  BTA_DM_API_ENABLE_TEST_MODE_EVT     */
    bta_dm_disable_test_mode, /*  BTA_DM_API_DISABLE_TEST_MODE_EVT    */
    bta_dm_execute_callback,  /*  BTA_DM_API_EXECUTE_CBACK_EVT        */

    bta_dm_remove_all_acl, /* BTA_DM_API_REMOVE_ALL_ACL_EVT */
    bta_dm_remove_device,  /* BTA_DM_API_REMOVE_DEVICE_EVT */
    bta_dm_hci_raw_command, /* BTA_DM_API_HCI_RAW_COMMAND_EVT */
    bta_dm_set_wifi_state, /* BTA_DM_API_SET_WIFI_STATE_EVT */
    bta_dm_power_back_off, /* BTA_DM_API_SET_PWR_BACKOFF_EVT */
    bta_dm_process_iot_report, /*BTA_DM_API_IOT_REPORT_EVT*/
    bta_dm_bredr_cleanup, /* BTA_DM_API_BREDR_CLEANUP_EVT */
    bta_dm_bredr_startup, /* BTA_DM_API_BREDR_STARTUP_EVT */
    bta_dm_reset_pairing_flag, /*BTA_DM_API_RST_PAIR_FLAG_EVT*/
    bta_dm_ble_subrate_request, /* BTA_DM_API_SUBRATE_REQ_EVT */
};

/* state machine action enumeration list */
enum {
  BTA_DM_API_SEARCH,                 /* 0 bta_dm_search_start */
  BTA_DM_API_SEARCH_CANCEL,          /* 1 bta_dm_search_cancel */
  BTA_DM_API_DISCOVER,               /* 2 bta_dm_discover */
  BTA_DM_INQUIRY_CMPL,               /* 3 bta_dm_inq_cmpl */
  BTA_DM_REMT_NAME,                  /* 4 bta_dm_rmt_name */
  BTA_DM_SDP_RESULT,                 /* 5 bta_dm_sdp_result */
  BTA_DM_SEARCH_CMPL,                /* 6 bta_dm_search_cmpl*/
  BTA_DM_FREE_SDP_DB,                /* 7 bta_dm_free_sdp_db */
  BTA_DM_DISC_RESULT,                /* 8 bta_dm_disc_result */
  BTA_DM_SEARCH_RESULT,              /* 9 bta_dm_search_result */
  BTA_DM_QUEUE_SEARCH,               /* 10 bta_dm_queue_search */
  BTA_DM_QUEUE_DISC,                 /* 11 bta_dm_queue_disc */
  BTA_DM_SEARCH_CLEAR_QUEUE,         /* 12 bta_dm_search_clear_queue */
  BTA_DM_SEARCH_CANCEL_CMPL,         /* 13 bta_dm_search_cancel_cmpl */
  BTA_DM_SEARCH_CANCEL_NOTIFY,       /* 14 bta_dm_search_cancel_notify */
  BTA_DM_SEARCH_CANCEL_TRANSAC_CMPL, /* 15 bta_dm_search_cancel_transac_cmpl */
  BTA_DM_DISC_RMT_NAME,              /* 16 bta_dm_disc_rmt_name */
  BTA_DM_API_DI_DISCOVER,            /* 17 bta_dm_di_disc */
  BTA_DM_CLOSE_GATT_CONN,            /* 18 bta_dm_close_gatt_conn */
  BTA_DM_QUEUE_SVC_DISC,             /* 19 bta_dm_queue_service_disc */
  BTA_DM_SEARCH_NUM_ACTIONS          /* 20 */
};

/* action function list */
const tBTA_DM_ACTION bta_dm_search_action[] = {

    bta_dm_search_start,               /* 0 BTA_DM_API_SEARCH */
    bta_dm_search_cancel,              /* 1 BTA_DM_API_SEARCH_CANCEL */
    bta_dm_discover,                   /* 2 BTA_DM_API_DISCOVER */
    bta_dm_inq_cmpl,                   /* 3 BTA_DM_INQUIRY_CMPL */
    bta_dm_rmt_name,                   /* 4 BTA_DM_REMT_NAME */
    bta_dm_sdp_result,                 /* 5 BTA_DM_SDP_RESULT */
    bta_dm_search_cmpl,                /* 6 BTA_DM_SEARCH_CMPL */
    bta_dm_free_sdp_db,                /* 7 BTA_DM_FREE_SDP_DB */
    bta_dm_disc_result,                /* 8 BTA_DM_DISC_RESULT */
    bta_dm_search_result,              /* 9 BTA_DM_SEARCH_RESULT */
    bta_dm_queue_search,               /* 10 BTA_DM_QUEUE_SEARCH */
    bta_dm_queue_disc,                 /* 11 BTA_DM_QUEUE_DISC */
    bta_dm_search_clear_queue,         /* 12 BTA_DM_SEARCH_CLEAR_QUEUE */
    bta_dm_search_cancel_cmpl,         /* 13 BTA_DM_SEARCH_CANCEL_CMPL */
    bta_dm_search_cancel_notify,       /* 14 BTA_DM_SEARCH_CANCEL_NOTIFY */
    bta_dm_search_cancel_transac_cmpl, /* 15 BTA_DM_SEARCH_CANCEL_TRANSAC_CMPL
                                          */
    bta_dm_disc_rmt_name,              /* 16 BTA_DM_DISC_RMT_NAME */
    bta_dm_di_disc,                    /* 17 BTA_DM_API_DI_DISCOVER */
    bta_dm_close_gatt_conn,
    bta_dm_queue_service_disc          /* 19 BTA_DM_QUEUE_SVC_DISC*/};

#define BTA_DM_SEARCH_IGNORE BTA_DM_SEARCH_NUM_ACTIONS
/* state table information */
#define BTA_DM_SEARCH_ACTIONS 2    /* number of actions */
#define BTA_DM_SEARCH_NEXT_STATE 2 /* position of next state */
#define BTA_DM_SEARCH_NUM_COLS 3   /* number of columns in state tables */

/* state table for listen state */
const uint8_t bta_dm_search_idle_st_table[][BTA_DM_SEARCH_NUM_COLS] = {

    /* Event                        Action 1
       Action 2                    Next State */
    /* API_SEARCH */ {BTA_DM_API_SEARCH, BTA_DM_SEARCH_IGNORE,
                      BTA_DM_SEARCH_ACTIVE},
    /* API_SEARCH_CANCEL */ {BTA_DM_SEARCH_CANCEL_NOTIFY, BTA_DM_SEARCH_IGNORE,
                             BTA_DM_SEARCH_IDLE},
    /* API_SEARCH_DISC */ {BTA_DM_API_DISCOVER, BTA_DM_SEARCH_IGNORE,
                           BTA_DM_DISCOVER_ACTIVE},
    /* INQUIRY_CMPL */ {BTA_DM_SEARCH_IGNORE, BTA_DM_SEARCH_IGNORE,
                        BTA_DM_SEARCH_IDLE},
    /* REMT_NAME_EVT */ {BTA_DM_SEARCH_IGNORE, BTA_DM_SEARCH_IGNORE,
                         BTA_DM_SEARCH_IDLE},
    /* SDP_RESULT_EVT */ {BTA_DM_FREE_SDP_DB, BTA_DM_SEARCH_IGNORE,
                          BTA_DM_SEARCH_IDLE},
    /* SEARCH_CMPL_EVT */ {BTA_DM_SEARCH_IGNORE, BTA_DM_SEARCH_IGNORE,
                           BTA_DM_SEARCH_IDLE},
    /* DISCV_RES_EVT */ {BTA_DM_SEARCH_IGNORE, BTA_DM_SEARCH_IGNORE,
                         BTA_DM_SEARCH_IDLE},
    /* API_DI_DISCOVER_EVT */ {BTA_DM_API_DI_DISCOVER, BTA_DM_SEARCH_IGNORE,
                               BTA_DM_SEARCH_ACTIVE},
    /* DISC_CLOSE_TOUT_EVT */
    {BTA_DM_CLOSE_GATT_CONN, BTA_DM_SEARCH_IGNORE, BTA_DM_SEARCH_IDLE}};
const uint8_t bta_dm_search_search_active_st_table[][BTA_DM_SEARCH_NUM_COLS] = {

    /* Event                        Action 1
       Action 2                    Next State */
    /* API_SEARCH */ {BTA_DM_SEARCH_IGNORE, BTA_DM_SEARCH_IGNORE,
                      BTA_DM_SEARCH_ACTIVE},
    /* API_SEARCH_CANCEL */ {BTA_DM_API_SEARCH_CANCEL, BTA_DM_SEARCH_IGNORE,
                             BTA_DM_SEARCH_CANCELLING},
    /* API_SEARCH_DISC */ {BTA_DM_QUEUE_DISC, BTA_DM_SEARCH_IGNORE,
                           BTA_DM_SEARCH_ACTIVE},
    /* INQUIRY_CMPL */ {BTA_DM_INQUIRY_CMPL, BTA_DM_SEARCH_IGNORE,
                        BTA_DM_SEARCH_ACTIVE},
    /* REMT_NAME_EVT */ {BTA_DM_REMT_NAME, BTA_DM_SEARCH_IGNORE,
                         BTA_DM_SEARCH_ACTIVE},
    /* SDP_RESULT_EVT */ {BTA_DM_SDP_RESULT, BTA_DM_SEARCH_IGNORE,
                          BTA_DM_SEARCH_ACTIVE},
    /* SEARCH_CMPL_EVT */ {BTA_DM_SEARCH_CMPL, BTA_DM_SEARCH_IGNORE,
                           BTA_DM_SEARCH_IDLE},
    /* DISCV_RES_EVT */ {BTA_DM_SEARCH_RESULT, BTA_DM_SEARCH_IGNORE,
                         BTA_DM_SEARCH_ACTIVE},
    /* API_DI_DISCOVER_EVT */ {BTA_DM_SEARCH_IGNORE, BTA_DM_SEARCH_IGNORE,
                               BTA_DM_SEARCH_ACTIVE},
    /* DISC_CLOSE_TOUT_EVT */
    {BTA_DM_CLOSE_GATT_CONN, BTA_DM_SEARCH_IGNORE, BTA_DM_SEARCH_ACTIVE}

};

const uint8_t
    bta_dm_search_search_cancelling_st_table[][BTA_DM_SEARCH_NUM_COLS] = {

        /* Event                        Action 1
           Action 2                    Next State */
        /* API_SEARCH */ {BTA_DM_QUEUE_SEARCH, BTA_DM_SEARCH_IGNORE,
                          BTA_DM_SEARCH_CANCELLING},
        /* API_SEARCH_CANCEL */ {BTA_DM_SEARCH_CLEAR_QUEUE,
                                 BTA_DM_SEARCH_CANCEL_NOTIFY,
                                 BTA_DM_SEARCH_CANCELLING},
        /* API_SEARCH_DISC */ {BTA_DM_QUEUE_DISC, BTA_DM_SEARCH_IGNORE,
                               BTA_DM_SEARCH_CANCELLING},
        /* INQUIRY_CMPL */ {BTA_DM_SEARCH_CANCEL_CMPL, BTA_DM_SEARCH_IGNORE,
                            BTA_DM_SEARCH_IDLE},
        /* REMT_NAME_EVT */ {BTA_DM_SEARCH_CANCEL_TRANSAC_CMPL,
                             BTA_DM_SEARCH_CANCEL_CMPL, BTA_DM_SEARCH_IDLE},
        /* SDP_RESULT_EVT */ {BTA_DM_SEARCH_CANCEL_TRANSAC_CMPL,
                              BTA_DM_SEARCH_CANCEL_CMPL, BTA_DM_SEARCH_IDLE},
        /* SEARCH_CMPL_EVT */ {BTA_DM_SEARCH_CANCEL_TRANSAC_CMPL,
                               BTA_DM_SEARCH_CANCEL_CMPL, BTA_DM_SEARCH_IDLE},
        /* DISCV_RES_EVT */ {BTA_DM_SEARCH_CANCEL_TRANSAC_CMPL,
                             BTA_DM_SEARCH_CANCEL_CMPL, BTA_DM_SEARCH_IDLE},
        /* API_DI_DISCOVER_EVT */ {BTA_DM_SEARCH_IGNORE, BTA_DM_SEARCH_IGNORE,
                                   BTA_DM_SEARCH_CANCELLING},
        /* DISC_CLOSE_TOUT_EVT */
        {BTA_DM_SEARCH_IGNORE, BTA_DM_SEARCH_IGNORE, BTA_DM_SEARCH_CANCELLING}};

const uint8_t bta_dm_search_disc_active_st_table[][BTA_DM_SEARCH_NUM_COLS] = {

    /* Event                        Action 1
       Action 2                    Next State */
    /* API_SEARCH */ {BTA_DM_QUEUE_SEARCH, BTA_DM_SEARCH_IGNORE,
                      BTA_DM_DISCOVER_ACTIVE},
    /* API_SEARCH_CANCEL */ {BTA_DM_SEARCH_CANCEL_NOTIFY, BTA_DM_SEARCH_IGNORE,
                             BTA_DM_SEARCH_CANCELLING},
    /* API_SEARCH_DISC */ {BTA_DM_QUEUE_SVC_DISC, BTA_DM_SEARCH_IGNORE,
                           BTA_DM_DISCOVER_ACTIVE},
    /* INQUIRY_CMPL */ {BTA_DM_SEARCH_IGNORE, BTA_DM_SEARCH_IGNORE,
                        BTA_DM_DISCOVER_ACTIVE},
    /* REMT_NAME_EVT */ {BTA_DM_DISC_RMT_NAME, BTA_DM_SEARCH_IGNORE,
                         BTA_DM_DISCOVER_ACTIVE},
    /* SDP_RESULT_EVT */ {BTA_DM_SDP_RESULT, BTA_DM_SEARCH_IGNORE,
                          BTA_DM_DISCOVER_ACTIVE},
    /* SEARCH_CMPL_EVT */ {BTA_DM_SEARCH_CMPL, BTA_DM_SEARCH_IGNORE,
                           BTA_DM_SEARCH_IDLE},
    /* DISCV_RES_EVT */ {BTA_DM_DISC_RESULT, BTA_DM_SEARCH_IGNORE,
                         BTA_DM_DISCOVER_ACTIVE},
    /* API_DI_DISCOVER_EVT */ {BTA_DM_SEARCH_IGNORE, BTA_DM_SEARCH_IGNORE,
                               BTA_DM_DISCOVER_ACTIVE},
    /* DISC_CLOSE_TOUT_EVT */
    {BTA_DM_SEARCH_IGNORE, BTA_DM_SEARCH_IGNORE, BTA_DM_DISCOVER_ACTIVE}};

typedef const uint8_t (*tBTA_DM_ST_TBL)[BTA_DM_SEARCH_NUM_COLS];

/* state table */
const tBTA_DM_ST_TBL bta_dm_search_st_tbl[] = {
    bta_dm_search_idle_st_table, bta_dm_search_search_active_st_table,
    bta_dm_search_search_cancelling_st_table,
    bta_dm_search_disc_active_st_table};

/*******************************************************************************
 *
 * Function         bta_dm_sm_disable
 *
 * Description     unregister BTA DM
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_sm_disable() { bta_sys_deregister(BTA_ID_DM); }

/*******************************************************************************
 *
 * Function         bta_dm_sm_execute
 *
 * Description      State machine event handling function for DM
 *
 *
 * Returns          void
 *
 ******************************************************************************/
bool bta_dm_sm_execute(BT_HDR* p_msg) {
  uint16_t event = p_msg->event & 0x00ff;

  APPL_TRACE_EVENT("bta_dm_sm_execute event:0x%x", event);

  /* execute action functions */
  if (event < BTA_DM_NUM_ACTIONS) {
    (*bta_dm_action[event])((tBTA_DM_MSG*)p_msg);
  }

  return true;
}

/*******************************************************************************
 *
 * Function         bta_dm_sm_search_disable
 *
 * Description     unregister BTA SEARCH DM
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_dm_search_sm_disable() { bta_sys_deregister(BTA_ID_DM_SEARCH); }

/*******************************************************************************
 *
 * Function         bta_dm_search_sm_execute
 *
 * Description      State machine event handling function for DM
 *
 *
 * Returns          void
 *
 ******************************************************************************/
bool bta_dm_search_sm_execute(BT_HDR* p_msg) {
  tBTA_DM_ST_TBL state_table;
  uint8_t action;
  int i;

  APPL_TRACE_EVENT("bta_dm_search_sm_execute state:%d, event:0x%x",
                   bta_dm_search_cb.state, p_msg->event);

  /* look up the state table for the current state */
  state_table = bta_dm_search_st_tbl[bta_dm_search_cb.state];

  bta_dm_search_cb.state =
      state_table[p_msg->event & 0x00ff][BTA_DM_SEARCH_NEXT_STATE];

  /* execute action functions */
  for (i = 0; i < BTA_DM_SEARCH_ACTIONS; i++) {
    action = state_table[p_msg->event & 0x00ff][i];
    if (action < BTA_DM_SEARCH_IGNORE) {
       APPL_TRACE_EVENT("bta_dm_search_sm_execute state:%d, action %d ",
                   bta_dm_search_cb.state, action);
      (*bta_dm_search_action[action])((tBTA_DM_MSG*)p_msg);
    } else {
      break;
    }
  }
  return true;
}
