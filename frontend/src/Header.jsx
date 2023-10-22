import React from "react";
import "./css/Header.css";

function Header({ type }) {
  if (type === "welcome") {
    return (
      <div className="header">
        <h1> </h1>
        <h1>Welcome!</h1>
        <p>
          This game will customize a series of questions depending on your music
          taste.{" "}
        </p>
        <p>
          You will select between two choices and gradually work your way up.
        </p>
        <p>Click below to login and begin the game!</p>
      </div>
    );
  }
  return (
    <div className="header">
      <h1>Who ranks higher in popularity?</h1>
    </div>
  );
}

export default Header;
