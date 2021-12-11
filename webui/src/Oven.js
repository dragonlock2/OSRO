import React from 'react';
import { 
  LineChart,
  Line,
  Legend,
  XAxis,
  YAxis,
  ResponsiveContainer,
} from 'recharts';

import './Oven.css';

// from CSS
const graph_blue = '#4d64ff';
const graph_red  = '#c62828';

// Sidebar Components
class Temp extends React.Component {
  render() {
    return (
      <div className='Info'>
        <p className='Label'>{this.props.name}</p>
        <p className='Data'>{this.props.temp.toFixed(1)}°C</p>
      </div>
    );
  }
}

class TempManual extends React.Component {
  constructor(props) {
    super(props);

    this.state = {
      selected: false,
    };
  }

  submit(e) {
    e.preventDefault();
    let temp = parseFloat(e.target[0].value);
    if (!Number.isNaN(temp)) {
      this.props.setTemp(temp);
    }
    this.setState({
      selected: false,
    });
  }

  render() {
    if (this.state.selected && this.props.isManual) {
      return (
        <div className='Info'>
          <p className='Label'>{this.props.name}</p>
          <form autoComplete='off' onSubmit={(e)=>this.submit(e)}>
            <input
              autoFocus
              id='TempManualInput'
              className='Input'
              placeholder='Got Temp?'
            />
          </form>
        </div>
      );
    }

    if (this.props.isManual) {
      return (
        <div className='Info'>
          <p className='Label'>{this.props.name}</p>
          <p
            className='Data'
            onClick={()=>this.setState({selected: true})}
          >
            {(this.props.running ? this.props.temp : 
                this.props.desiredTemp).toFixed(1)}°C
          </p>
        </div>
      );
    }

    return (
      <div className='Info'>
        <p className='Label'>{this.props.name}</p>
        <p className='Data'>{this.props.temp.toFixed(1)}°C</p>
      </div>
    );
  }
}

class Dropdown extends React.Component {
  constructor(props) {
    super(props);

    this.state = {
      selected: true,
    }
  }

  render() {
    if (this.state.selected) {
      return (
        <div className='Info'>
          <p className='Label'>Profile:</p>
          <p
            onClick={()=>this.setState({selected: false})}
            className='Data'
          >
            {this.props.idx === -1 ? 'None' :
              this.props.profiles[this.props.idx].name}
          </p>
        </div>
      );
    }

    return (
      <div className='Info'>
        <p className='Label'>Select one:</p>
        {this.props.profiles.map((prof, idx)=>
          <p 
            key={prof.name}
            onClick={()=>{
              this.props.selectIdx(idx);
              this.setState({selected: true});
            }}
            className='Data'
          >
            {prof.name}
          </p>
        )}
      </div>
    );
  }
}

class Buttons extends React.Component {
  render() {
    return (
      <div className='Buttons'>
        <button
          style={{opacity: (this.props.running ? 0.5 : 1)}}
          className='Start'
          onClick={this.props.start}
        >
          Start
        </button>
        <button className='Stop' onClick={this.props.stop}>Stop</button>
      </div>
    );
  }
}

// Oven Components
class Graph extends React.Component {
  constructor(props) {
    super(props);

    this.ticks = ((start, stop, step)=>{
      let arr = [];
      for (let i = 0; i <= stop; i += step) {
        arr.push(i);
      }
      return arr;
    })(0, this.props.maxTime, 20);

    const times = Array.from(Array(this.props.maxTime).keys());
    const target = times.map((val, idx)=>{
      return Math.random()*300;
    });
    const current = times.map((val, idx)=>{
      return Math.random()*300;
    });

    this.state = {
      times: times,
      target: target,
      current: current,
    };
  }

  componentDidMount() {
    // this.timer = setInterval(()=>this.tick(), 1000);
  }

  componentWillUnmount() {
    // clearInterval(this.timer);
  }

  tick() {
    const target = this.state.target;
    const current = this.state.current;

    this.setState({
      target: [...target.slice(1), Math.random()*300],
      current: [...current.slice(1), Math.random()*300],
    });
  }

  render() {
    const data = this.state.times.map((val, idx)=>{
      return {
        time: idx,
        target: this.state.target[idx],
        current: this.state.current[idx],
      };
    });

    return (
      <div className='Graph'>
        <ResponsiveContainer>
          <LineChart data={data} margin={{left: -4}}>
            <Legend />
            <XAxis
              dataKey='time'
              ticks={this.ticks}
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
              domain={[0, 300]}
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
              type='linear'
              dataKey='target'
              stroke={graph_blue}
              strokeWidth={2}
              dot={false}
              isAnimationActive={false}
            />
            <Line
              name='Current'
              type='linear'
              dataKey='current'
              stroke={graph_red}
              strokeWidth={2}
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
  constructor(props) {
    super(props);

    this.state = {
      idx: -1,
      temp: 25.0,
    };
  }

  setTemp(temp) {
    this.setState({temp:temp});
    if (this.props.running && (this.state.idx === 0)) {
      this.props.start(0, temp);
    }
  }

  render() {
    return (
      <div className='Sidebar'>
        <Temp name='Current:' temp={this.props.current_temp} />
        <TempManual
          name='Target:'
          temp={this.props.target_temp}
          desiredTemp={this.state.temp}
          setTemp={(temp)=>this.setTemp(temp)}
          isManual={this.state.idx === 0}
          running={this.props.running}
        />
        <Dropdown
          profiles={this.props.profiles}
          idx={this.state.idx}
          selectIdx={(idx)=>this.setState({idx:idx})}
        />
        <Buttons
          running={this.props.running || (this.state.idx === -1)}
          start={()=>this.props.start(this.state.idx, this.state.temp)}
          stop={()=>this.props.stop()}
        />
      </div>
    );
  }
}

class Oven extends React.Component {
  constructor(props) {
    super(props);

    this.state = {
      current_temp: 0.0,
      target_temp: 0.0,
      running: false,
      profiles: [],
    };
  }

  componentDidMount() {
    fetch('/profiles')
      .then((res)=>res.json())
      .then((json)=>this.setState({profiles: json.profiles}));

    this.timer = setInterval(()=>this.tick(), 500);
    this.tick();
  }

  componentWillUnmount() {
    clearInterval(this.timer);
  }

  render() {
    return (
      <div className='Oven'>
        <Graph maxTime={300} />
        <Sidebar
          current_temp={this.state.current_temp}
          target_temp={this.state.target_temp}
          running={this.state.running}
          profiles={this.state.profiles}
          start={this.start}
          stop={this.stop}
        />
      </div>
    );
  }

  tick() {
    fetch('/temps')
      .then((res)=>res.json())
      .then((json)=>this.setState({
        current_temp: json.current,
        target_temp: json.target,
        running: json.running,
      }));
  }

  start(idx, temp) {
    const requestOptions = {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({idx:idx,temp:temp}),
    };
    fetch('/start', requestOptions);
  }

  stop() {
    const requestOptions = {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: '',
    };
    fetch('/stop', requestOptions);
  }
}

export default Oven;
