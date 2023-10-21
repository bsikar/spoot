import React from 'react'
import {useNavigate } from 'react-router-dom';
import './Button.css';


function Button() {
  const handleClick = () => { window.location.href = 'http://localhost:5555/login'; }
  return (
    <button onClick={handleClick} class="button" role="button">Login</button>
  )
}

export default Button