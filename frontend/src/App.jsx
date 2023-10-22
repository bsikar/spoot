import React from 'react';
import './App.css';
import Button from './components/Button/Button';
import Header from './components/Header/Header';
import Choice from './components/Choice/Choice';
import { RestartButton } from './components/Button/Button';
import { BrowserRouter as Router, Route, Routes, useNavigate } from 'react-router-dom';




function App() {
  return (
    <Router>
      <Routes>
        <Route path="/" element={<LoginPage />} />
        <Route path="/play" element={<PlayPage />} />
      </Routes>
    </Router>
  );
}
export default App;

function LoginPage() {
  const navigate = useNavigate();

  const handleButtonClick = () => {
    navigate('/play');
  };

  return (
    <div>
      <Header type="welcome"/>
      {/* When the button is pressed you will get redirected to the PlayPage */}
      <Button onClick={handleButtonClick} />
    </div>
  );
}

function PlayPage() {
  return (
    <div>
      <p><Header type="question"/></p>
      <br />
      <p>
        <Choice idx={0} />
        <Choice idx={1}/>
      </p>
    </div>
  );
}
