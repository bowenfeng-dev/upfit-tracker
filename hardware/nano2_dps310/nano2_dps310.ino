#include <Wire.h>
#include <nRF5x_BLE_API.h>
#include "ifx_dps310.h"

#define DEVICE_NAME "DPS310_NANO2"
#define TXRX_BUF_LEN 20

BLE ble;
Ticker ticker1s;

// Service UUID
static const uint8_t service1_uuid[]         = {0x71, 0x3D, 0, 0, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};
// Used in advertisement
static const uint8_t service1_uuid_rev[]    = {0x1E, 0x94, 0x8D, 0xF1, 0x48, 0x31, 0x94, 0xBA, 0x75, 0x4C, 0x3E, 0x50, 0, 0, 0x3D, 0x71};

// Characteristics UUID
static const uint8_t service1_chars1_uuid[]  = {0x71, 0x3D, 0, 2, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};

// Notify characteristic definition
uint8_t chars3_value[TXRX_BUF_LEN] = {0};
GattCharacteristic  characteristic3(service1_chars1_uuid, chars3_value, 4, TXRX_BUF_LEN, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
GattCharacteristic *uartChars[] = {&characteristic3};

//Create service
GattService         customService(service1_uuid, uartChars, sizeof(uartChars) / sizeof(GattCharacteristic *));

/** @brief  Disconnect callback handle
 *
 *  @param[in] *params   params->handle : connect handle
 *                       params->reason : CONNECTION_TIMEOUT                          = 0x08,
 *                                        REMOTE_USER_TERMINATED_CONNECTION           = 0x13,
 *                                        REMOTE_DEV_TERMINATION_DUE_TO_LOW_RESOURCES = 0x14,  // Remote device terminated connection due to low resources.
 *                                        REMOTE_DEV_TERMINATION_DUE_TO_POWER_OFF     = 0x15,  // Remote device terminated connection due to power off.
 *                                        LOCAL_HOST_TERMINATED_CONNECTION            = 0x16,
 *                                        CONN_INTERVAL_UNACCEPTABLE                  = 0x3B,
 */
 void disconnectionCallBack(const Gap::DisconnectionCallbackParams_t *params) {
  Serial.print("Disconnected hande : ");
  Serial.println(params->handle, HEX);
  Serial.print("Disconnected reason : ");
  Serial.println(params->reason, HEX);
  Serial.println("Restart advertising ");
  ble.startAdvertising();
}

/** @brief  Connection callback handle
 *
 *  @param[in] *params   params->handle : The ID for this connection
 *                       params->role : PERIPHERAL  = 0x1, // Peripheral Role
 *                                      CENTRAL     = 0x2, // Central Role.
 *                       params->peerAddrType : The peer's BLE address type
 *                       params->peerAddr : The peer's BLE address
 *                       params->ownAddrType : This device's BLE address type
 *                       params->ownAddr : This devices's BLE address
 *                       params->connectionParams->minConnectionInterval
 *                       params->connectionParams->maxConnectionInterval
 *                       params->connectionParams->slaveLatency
 *                       params->connectionParams->connectionSupervisionTimeout
 */
void connectionCallBack( const Gap::ConnectionCallbackParams_t *params ) {
  uint8_t index;
  if(params->role == Gap::PERIPHERAL) {
    Serial.println("Peripheral ");
  }

  Serial.print("The conn handle : ");
  Serial.println(params->handle, HEX);
  Serial.print("The conn role : ");
  Serial.println(params->role, HEX);

  Serial.print("The peerAddr type : ");
  Serial.println(params->peerAddrType, HEX);
  Serial.print("  The peerAddr : ");
  for(index=0; index<6; index++) {
    Serial.print(params->peerAddr[index], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");

  Serial.print("The ownAddr type : ");
  Serial.println(params->ownAddrType, HEX);
  Serial.print("  The ownAddr : ");
  for(index=0; index<6; index++) {
    Serial.print(params->ownAddr[index], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");

  Serial.print("The min connection interval : ");
  Serial.println(params->connectionParams->minConnectionInterval, HEX);
  Serial.print("The max connection interval : ");
  Serial.println(params->connectionParams->maxConnectionInterval, HEX);
  Serial.print("The slaveLatency : ");
  Serial.println(params->connectionParams->slaveLatency, HEX);
  Serial.print("The connectionSupervisionTimeout : ");
  Serial.println(params->connectionParams->connectionSupervisionTimeout, HEX);
}

volatile bool updatePressure = false;

/**
 * @brief  Timer task callback handle
 */
void task_handle(void) {
  updatePressure = true;
}


/**
 * @brief  Set advertisement
 */
void setAdvertisement(void) {
  // A list of Advertising Data types commonly used by peripherals.
  //   FLAGS                              = 0x01, // Flags, refer to GapAdvertisingData::Flags_t.
  //   INCOMPLETE_LIST_16BIT_SERVICE_IDS  = 0x02, // Incomplete list of 16-bit Service IDs.
  //   COMPLETE_LIST_16BIT_SERVICE_IDS    = 0x03, // Complete list of 16-bit Service IDs.
  //   INCOMPLETE_LIST_32BIT_SERVICE_IDS  = 0x04, // Incomplete list of 32-bit Service IDs (not relevant for Bluetooth 4.0).
  //   COMPLETE_LIST_32BIT_SERVICE_IDS    = 0x05, // Complete list of 32-bit Service IDs (not relevant for Bluetooth 4.0).
  //   INCOMPLETE_LIST_128BIT_SERVICE_IDS = 0x06, // Incomplete list of 128-bit Service IDs.
  //   COMPLETE_LIST_128BIT_SERVICE_IDS   = 0x07, // Complete list of 128-bit Service IDs.
  //   SHORTENED_LOCAL_NAME               = 0x08, // Shortened Local Name.
  //   COMPLETE_LOCAL_NAME                = 0x09, // Complete Local Name.
  //   TX_POWER_LEVEL                     = 0x0A, // TX Power Level (in dBm).
  //   DEVICE_ID                          = 0x10, // Device ID.
  //   SLAVE_CONNECTION_INTERVAL_RANGE    = 0x12, // Slave Connection Interval Range.
  //   LIST_128BIT_SOLICITATION_IDS       = 0x15, // List of 128 bit service UUIDs the device is looking for.
  //   SERVICE_DATA                       = 0x16, // Service Data.
  //   APPEARANCE                         = 0x19, // Appearance, refer to GapAdvertisingData::Appearance_t.
  //   ADVERTISING_INTERVAL               = 0x1A, // Advertising Interval.
  //   MANUFACTURER_SPECIFIC_DATA         = 0xFF  // Manufacturer Specific Data.

  // AD_Type_Flag : LE_LIMITED_DISCOVERABLE = 0x01, Peripheral device is discoverable for a limited period of time
  //                LE_GENERAL_DISCOVERABLE = 0x02, Peripheral device is discoverable at any moment
  //                BREDR_NOT_SUPPORTED     = 0x03, Peripheral device is LE only
  //                SIMULTANEOUS_LE_BREDR_C = 0x04, Not relevant - central mode only
  //                SIMULTANEOUS_LE_BREDR_H = 0x05, Not relevant - central mode only
  ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
  // Add short name to advertisement
  ble.accumulateAdvertisingPayload(GapAdvertisingData::SHORTENED_LOCAL_NAME,(const uint8_t *)"NANO310", 4);
  // Add complete 128bit_uuid to advertisement
  ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS,(const uint8_t *)service1_uuid_rev, sizeof(service1_uuid_rev));
  // Add complete device name to scan response data
  ble.accumulateScanResponse(GapAdvertisingData::COMPLETE_LOCAL_NAME,(const uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME) - 1);
}

void updateBlePressure() {
  long int pressure = readPressure();
  if (pressure == 0L) {
    Serial.println("pressure = 0");
    return;
  }

  // if true, saved to attribute, no notification or indication is generated.
  //ble.updateCharacteristicValue(characteristic3.getValueAttribute().getHandle(), (uint8_t *)&value, 2, true);
  // if false or ignore, notification or indication is generated if permit.
  ble.updateCharacteristicValue(characteristic3.getValueAttribute().getHandle(), (uint8_t *)&pressure, 4);
}

long int readPressure() {
  long int pressure;
  int oversampling = 7;
  int ret;

  //Pressure measurement behaves like temperature measurement
  //ret = ifxDps310.measurePressureOnce(pressure);
  ret = ifxDps310.measurePressureOnce(pressure, oversampling);
  if (ret != 0)
  {
    //Something went wrong.
    //Look at the library code for more information about return codes
    Serial.print("FAIL! ret = ");
    Serial.println(ret);
    return 0;
  }

  Serial.println(pressure); // Pascal
  return pressure;
}

void setup() {
  Serial.begin(9600);
  delay(5000);
  Serial.println("DPS310-NANO starts up");

  Serial.print("Initializing DPS310...");
  //Call begin to initialize ifxDps310
  //The parameter 0x76 is the bus address. The default address is 0x77 and does not need to be given.
  ifxDps310.begin(Wire, 0x77);
  Serial.print("...");

  // IMPORTANT NOTE
  //If you face the issue that the DPS310 indicates a temperature around 60 °C although it should be around 20 °C (room temperature), you might have got an IC with a fuse bit problem
  //Call the following function directly after begin() to resolve this issue (needs only be called once after startup)
  ifxDps310.correctTemp();
  Serial.println(" DONE");

  Serial.print("Initializing BLE...");
  ble.init();
  Serial.print("...");
  ble.onConnection(connectionCallBack);
  ble.onDisconnection(disconnectionCallBack);
  setAdvertisement();
  Serial.print("...");
  // set adv_type(enum from 0)
  //    ADV_CONNECTABLE_UNDIRECTED
  //    ADV_CONNECTABLE_DIRECTED
  //    ADV_SCANNABLE_UNDIRECTED
  //    ADV_NON_CONNECTABLE_UNDIRECTED
  ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
  // add service
  ble.addService(customService);
  // set device name
  ble.setDeviceName((const uint8_t *)DEVICE_NAME);
  // set tx power,valid values are -40, -20, -16, -12, -8, -4, 0, 4
  ble.setTxPower(4);
  // set adv_interval, 100ms in multiples of 0.625ms.
  ble.setAdvertisingInterval(160);
  // set adv_timeout, in seconds
  ble.setAdvertisingTimeout(0);
  // ger BLE stack version
  Serial.print("BLE stack verison is : ");
  Serial.println(ble.getVersion());

  // start advertising
  ble.startAdvertising();

  Serial.println("Init complete!");

  delay(1000);

  ticker1s.attach(task_handle, 0.2);
}

void loop() {
  if (updatePressure) {
    updatePressure = false;
    updateBlePressure();
  }
  ble.waitForEvent();
}


