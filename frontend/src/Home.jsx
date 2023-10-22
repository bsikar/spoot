import React from "react";
import Header from "./Header";
import "./css/Home.css";
import "./css/Button.css";

const Home = () => {
  const handleButtonClick = () => {
    window.location.href = "http://localhost:5555/login";
  };

  return (
    <div>
      <Header type="Welcome" />
      <button class="welcomeButton" onClick={handleButtonClick}>
        Go to Play
      </button>
    </div>
  );
};

export default Home;
