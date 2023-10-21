import React from 'react';
import './App.css';
import Button from './components/Button/Button';
import Header from './components/Header/Header';
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
      <Header />
      {/* When the button is pressed you will get redirected to the PlayPage */}
      <Button onClick={handleButtonClick} />
    </div>
  );
}

function PlayPage() {
  return (
    <div>
      <p>meow :3</p>
    </div>
  );
}