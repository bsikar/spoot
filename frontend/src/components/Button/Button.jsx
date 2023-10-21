import React from 'react'
import './Button.css';

function handleClick() {
  window.open('http://localhost:5555/login')
}

function Button() {
  return (
    <button onClick={handleClick} class="button" role="button">Login</button>
  )
}

export default Button