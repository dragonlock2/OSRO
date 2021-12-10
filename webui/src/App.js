import React from 'react';
import './App.css';

import Oven from './Oven.js';

class App extends React.Component {
  render() {
    return (
      <div className='App'>
        <p className='Title'>OSRO v1.0</p>
        <Oven />
      </div>
    );
  }
}

export default App;
