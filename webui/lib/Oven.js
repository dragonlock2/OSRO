import React from 'react';
import { View, Text, Pressable, Dimensions } from 'react-native';
import { Picker } from '@react-native-picker/picker';

import { LineChart } from "react-native-chart-kit";

import Palette from './Palette.js'

// SideBar Components
class Temp extends React.Component {
  render() {
    return (
      <View style={{
        flexDirection: 'column',
        padding: 12,
        borderWidth: 2,
        borderRadius: 16,
        marginBottom: 16,
        borderColor: Palette['200'],
        backgroundColor: Palette['900'],
      }}>
        <Text style={{
          color: Palette['200'],
          fontSize: 12,
          textAlign: 'left',
        }}>
          {this.props.name}
        </Text>
        <Text style={{
          color: Palette['200'],
          fontSize: 24,
          textAlign: 'left',
        }}>
          {this.props.temp}°C
        </Text>
      </View>
    );
  }
}

class Button extends React.Component {
  render() {
    return (
      <Pressable style={{
        flex: 0.5,
        backgroundColor: this.props.color,
        borderColor: Palette['200'],
        padding: 12,
        borderWidth: 2,
        borderRadius: 16,
        marginLeft: this.props.ml || 0,
        marginRight: this.props.mr || 0,
      }} onPress={()=>console.log(this.props.name)}>
        <Text style={{
          color: Palette['200'],
          fontSize: 24,
          textAlign: 'center',
        }}>
          {this.props.name}
        </Text>
      </Pressable>
    );
  }
}

class Buttons extends React.Component {
  render() {
    return (
      <View style={{
        flexDirection: 'row',
        marginBottom: 16,
      }}>
        <Button color={Palette['confirm']} name='Start' mr={8} />
        <Button color={Palette['remove']} name='Stop' ml={8} />
      </View>
    );
  }
}

class Dropdown extends React.Component {
  render() {
    return (
      <View style={{
        flexDirection: 'column',
        padding: 12,
        borderWidth: 2,
        borderRadius: 16,
        marginBottom: 16,
        borderColor: Palette['200'],
        backgroundColor: Palette['900'],
      }}>
        <Text style={{
          color: Palette['200'],
          fontSize: 12,
          textAlign: 'left',
        }}>
          Profile:
        </Text>
        <Picker
          style={{
            color: Palette['200'],
            backgroundColor: Palette['900'],
            borderColor: Palette['900'],
            fontSize: 24,
          }}
          onValueChange={(val, idx)=>console.log(idx, val)}
          enabled={true}
        >
          <Picker.Item label='SAC305' value='SAC305' />
          <Picker.Item label='Sn63/Pb37' value='Sn63/Pb37' />
          <Picker.Item label='Manual' value='Manual' />
        </Picker>
      </View>
    );
  }
}

// Oven Components
class Graph extends React.Component {
  render() {
    return (
      <View style={{
        flex: 0.8,
        borderWidth: 2,
        borderRadius: 16,
        borderColor: Palette['200'],
        backgroundColor: Palette['900'],
        flexDirection: 'row'
      }}>
        <LineChart
          data={{
            labels: ["January", "February", "March", "April", "May", "June"],
            datasets: [
              {
                data: [
                  Math.random() * 100,
                  Math.random() * 100,
                  Math.random() * 100,
                  Math.random() * 100,
                  Math.random() * 100,
                  Math.random() * 100
                ]
              }
            ]
          }}
          width={Dimensions.get("window").width * 0.75} // from react-native
          height={Dimensions.get("window").height * 0.65}
          yAxisLabel="$"
          yAxisSuffix="k"
          yAxisInterval={1} // optional, defaults to 1
          chartConfig={{
            backgroundColor: "#e26a00",
            backgroundGradientFrom: "#fb8c00",
            backgroundGradientTo: "#ffa726",
            decimalPlaces: 2, // optional, defaults to 2dp
            color: (opacity = 1) => `rgba(255, 255, 255, ${opacity})`,
            labelColor: (opacity = 1) => `rgba(255, 255, 255, ${opacity})`,
            style: {
              borderRadius: 16
            },
            propsForDots: {
              r: "6",
              strokeWidth: "2",
              stroke: "#ffa726"
            }
          }}
          bezier
          style={{
            marginVertical: 0,
            borderRadius: 16,
            borderWidth: 2,
            borderColor: Palette['200']
          }}
        />
      </View>
    );
  }
}

class SideBar extends React.Component {
  render() {
    return (
      <View style={{
        flex: 0.2,
        flexDirection: 'column',
        marginLeft: 16,
      }}>
        <Temp name="Current:" temp={25.5}/>
        <Temp name="Target:" temp={220.0}/>
        <Dropdown />
        <Buttons />
      </View>
    );
  }
}

class Oven extends React.Component {
  render() {
    return (
      <View style={{
        flexDirection: 'row',
        height: '70%',
      }}>
        <Graph />
        <SideBar />
      </View>
    );
  }
}

export default Oven;
