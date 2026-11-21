// Zigbee2MQTT (2.x) external converter for this firmware (adds the new settings).
// Copy to <z2m data>/external_converters/ and restart Zigbee2MQTT.
// It replaces the built-in DIYRuZ_Geiger definition, keeping the same entity names.
import * as m from 'zigbee-herdsman-converters/lib/modernExtend';

const BOOLEAN = 0x10;
const UINT8 = 0x20;
const UINT16 = 0x21;
const UINT32 = 0x23;
const ENUM8 = 0x30;

const config = 'msIlluminanceLevelSensing';
const measurement = 'msIlluminanceMeasurement';

const switchAttr = (name, id, description) =>
    m.binary({
        name,
        cluster: config,
        attribute: {ID: id, type: BOOLEAN},
        valueOn: ['ON', 1],
        valueOff: ['OFF', 0],
        description,
        access: 'ALL',
        entityCategory: 'config',
    });

export default {
    zigbeeModel: ['DIYRuZ_Geiger'],
    model: 'DIYRuZ_Geiger',
    vendor: 'DIYRuZ',
    description: 'Geiger counter',
    extend: [
        m.numeric({
            name: 'radioactive_events_per_minute',
            cluster: measurement,
            attribute: {ID: 0xf001, type: UINT16},
            unit: 'cpm',
            description: 'Registered particles per minute',
            access: 'STATE_GET',
            reporting: {min: 10, max: 600, change: 1},
        }),
        m.numeric({
            name: 'radiation_dose_per_hour',
            cluster: measurement,
            attribute: {ID: 0xf002, type: UINT32},
            unit: 'μR/h',
            description: 'Dose rate',
            access: 'STATE_GET',
            reporting: {min: 10, max: 600, change: 1},
        }),
        switchAttr('led_feedback', 0xf001, 'Blink the LED on every particle'),
        switchAttr('buzzer_feedback', 0xf002, 'Alarm sound when alert_threshold is exceeded'),
        switchAttr('buzzer_clicks', 0xf006, 'Click on every particle'),
        m.numeric({
            name: 'sensitivity',
            cluster: config,
            attribute: {ID: 0xf000, type: UINT16},
            description: 'Tube sensitivity (used when sensors_type is "Others")',
            valueMin: 0,
            valueMax: 1000,
            access: 'ALL',
            entityCategory: 'config',
        }),
        m.numeric({
            name: 'sensors_count',
            cluster: config,
            attribute: {ID: 0xf003, type: UINT8},
            description: 'Number of installed tubes',
            valueMin: 1,
            valueMax: 50,
            access: 'ALL',
            entityCategory: 'config',
        }),
        m.enumLookup({
            name: 'sensors_type',
            cluster: config,
            attribute: {ID: 0xf004, type: ENUM8},
            lookup: {'СБМ-20/СТС-5/BOI-33': 0, 'СБМ-19/СТС-6': 1, Others: 2},
            description: 'Tube type',
            access: 'ALL',
            entityCategory: 'config',
        }),
        m.numeric({
            name: 'alert_threshold',
            cluster: config,
            attribute: {ID: 0xf005, type: UINT32},
            unit: 'μR/h',
            description: 'Alarm level',
            valueMin: 0,
            valueMax: 10000,
            access: 'ALL',
            entityCategory: 'config',
        }),
    ],
};
