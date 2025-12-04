import QtQuick 2.15
import QtCharts 2.15
import QtQuick.Controls 2.15

Rectangle {
    width: 600
    height: 400

    ChartView {
        id: chartView
        anchors.fill: parent
        title: "Chart with Category X-Axis"
        antialiasing: true

        CategoryAxis {
            id: categoryAxisX
            titleText: "Category"
            startValue: 0
             labelsPosition: CategoryAxis.AxisLabelsPositionOnValue
        }

        ValueAxis {
            id: valueAxisY
            titleText: "Value"
            min: 0
            max: 10
        }

        LineSeries {
            id: lineSeries
            name: "Data Series"
        }

        Component.onCompleted: {
            lineSeries.axisX = categoryAxisX;
            lineSeries.axisY = valueAxisY;

            var categoryLabels = ["Jan", "Feb", "Mar", "Apr", "May", "Jun"];
            var values = [4, 6, 3, 8, 5, 7];

            var currentValue = 0;

            for (var i = 0; i < categoryLabels.length; i++) {
                categoryAxisX.append(categoryLabels[i], i );
            }
            for (var j = 0; j < values.length; j++) {
                lineSeries.append(j   , values[j]);
            }
            categoryAxisX.max =6
        }
    }
}
