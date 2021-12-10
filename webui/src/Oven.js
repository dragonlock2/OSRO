import React from 'react';
import { 
  LineChart,
  Line,
  XAxis,
  YAxis,
  ResponsiveContainer,
  Legend
} from 'recharts';

import './Oven.css';

// Sidebar Components
class Temp extends React.Component {
  render() {
    return (
      <div className='Info'>
        <p className='Label'>{this.props.name}</p>
        <p className='Data'>{this.props.temp}°C</p>
      </div>
    );
  }
}

class Dropdown extends React.Component {
  constructor(props) {
    super(props);

    this.state = {
      selected: true,
      idx: -1,
      options: [
        'SAC305',
        'Sn63/Pb37',
        'Manual',
      ],
    }
  }

  click(idx) {
    console.log(idx);

    this.setState({
      selected: !this.state.selected,
      idx: idx,
    });
  }

  render() {
    if (this.state.selected) {
      return (
        <div className='Info'>
          <p className='Label'>Profile:</p>
          <p
            onClick={()=>this.click(-1)}
            className='Data'
          >
            {this.state.idx === -1 ? 'None' : this.state.options[this.state.idx]}
          </p>
        </div>
      );
    }

    return (
      <div className='Info'>
        <p className='Label'>Select one:</p>
        {this.state.options.map((prof, idx)=>
          <p 
            key={prof}
            onClick={()=>this.click(idx)}
            className='Data'
          >
            {prof}
          </p>
        )}
      </div>
    );
  }
}

class Buttons extends React.Component {
  start() {
    console.log('start');
  }

  stop() {
    console.log('stop');
  }

  render() {
    return (
      <div className='Buttons'>
        <button className='Start' onClick={this.start}>Start</button>
        <button className='Stop' onClick={this.stop}>Stop</button>
      </div>
    );
  }
}

// Oven Components
class Graph extends React.Component {
  render() {
    const times = Array.from(Array(240).keys());
    const data1 = times.map((val, idx)=>{
      return Math.random()*300;
    });
    const data2 = times.map((val, idx)=>{
      return Math.random()*300;
    });

    const data = times.map((val, idx)=>{
      return {
        time: idx,
        target: data1[idx],
        actual: data2[idx],
      };
    });

    return (
      <div className='Graph'>
        <ResponsiveContainer>
          <LineChart data={data} margin={{left: -4}}>
            <Legend />
            <XAxis
              dataKey='time'
              type='number'
              domain={['dataMin', 'dataMax']}
              scale='linear'
              unit='s'
              stroke='#cfcfcf'
              strokeWidth={2}
              tick={{fontSize: '1rem', fill: '#cfcfcf'}}
              tickCount={8}
              padding={{right: 16}}
            />
            <YAxis
              type='number'
              domain={['dataMin-10', 'dataMax+10']}
              scale='linear'
              unit='°C'
              stroke='#cfcfcf'
              strokeWidth={2}
              tick={{fontSize: '1rem', fill: '#cfcfcf'}}
              tickCount={8}
              padding={{top: 16}}
            />
            <Line
              name='Target'
              type='monotone'
              dataKey='target'
              stroke='#4d64ff'
              strokeWidth={3}
              dot={false}
              isAnimationActive={false}
            />
            <Line
              name='Actual'
              type='monotone'
              dataKey='actual'
              stroke='#c62828'
              strokeWidth={3}
              dot={false}
              isAnimationActive={false}
            />
          </LineChart>
        </ResponsiveContainer>
      </div>
    );
  }
}

class Sidebar extends React.Component {
  render() {
    return (
      <div className='Sidebar'>
        <Temp name='Current:' temp={26.1}/>
        <Temp name='Target:' temp={220.3}/>
        <Dropdown />
        <Buttons />
      </div>
    );
  }
}

class Oven extends React.Component {
  render() {
    return (
      <div className='Oven'>
        <Graph />
        <Sidebar />
      </div>
    );
  }
}

export default Oven;