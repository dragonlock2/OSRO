import React from 'react';
import { Text, View } from 'react-native';

import Palette from './lib/Palette.js';
import Oven from './lib/Oven.js';

class App extends React.Component {
  render() {
    return (
      <View style={{
        backgroundColor: Palette['bg'],
        padding: 16,
        flex: 1,
        flexDirection: 'column',
      }}>
        <Text style={{
          color: Palette['200'],
          fontSize: 24,
          fontWeight: 'bold',
          textAlign: 'left',
          marginBottom: 16,
        }}>
          OSRO v1.0
        </Text>
        <Oven />
      </View>
    );
  }
}

export default App;
