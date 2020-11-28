import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQml 2.12

Item {
    id: sliderSpinbox
    property real value: 0.0
    property real from: 0.0
    property real to: 100.0
    property real stepSize: 1.0

    implicitWidth: slider.implicitWidth + spinbox.implicitWidth
    implicitHeight: Math.max(slider.implicitHeight, spinbox.implicitHeight)

    RowLayout {
        Slider {
            id: slider
            stepSize: sliderSpinbox.stepSize
            from: sliderSpinbox.from
            to: sliderSpinbox.to

            Binding {
                target: sliderSpinbox
                property: 'value'
                value: slider.value
            }

            Binding {
                target: slider
                property: 'value'
                value: sliderSpinbox.value
            }
        }

        SpinBox {
            id: spinbox
            property int decimals: 2

            from: sliderSpinbox.from * 100
            to: sliderSpinbox.to * 100
            stepSize: sliderSpinbox.stepSize * 100
            editable: true

            validator: DoubleValidator {
                bottom: Math.min(spinbox.from, spinbox.to)
                top:  Math.max(spinbox.from, spinbox.to)
            }

            textFromValue: function(value, locale) {
                return Number(value / 100).toLocaleString(locale, 'f', spinbox.decimals)
            }

            valueFromText: function(text, locale) {
                return Number.fromLocaleString(locale, text) * 100
            }

            Binding {
                target: sliderSpinbox
                property: 'value'
                value: spinbox.value / 100
            }

            Binding {
                target: spinbox
                property: 'value'
                value: sliderSpinbox.value * 100
            }
        }

    }
}
