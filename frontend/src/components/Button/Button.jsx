import React from 'react'
import {useNavigate } from 'react-router-dom';
import './Button.css';

// this file will hold various types of buttons used throughout the program 

function Button() {
  const handleClick = () => { window.location.href = 'http://localhost:5555/login'; }
  return (
    <button onClick={handleClick} class="button" role="button">Login</button>
  )
}
export default Button

export function RestartButton() {
  return (
    <button class="button" role="button">Try Again!</button>
  )
}