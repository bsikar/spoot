import { useNavigate } from "react-router-dom";
import { useLocation } from "react-router-dom";
import "./css/Results.css";
import "./css/Button.css";

const Results = () => {
  const location = useLocation();
  const result = location.state?.res;
  const navigate = useNavigate();

  const message =
    result === true
      ? "Congratulations you won!"
      : "Aww, better luck next time!";

  const handlePlayAgain = (difficulty) => {
    navigate("/play", { state: { difficulty } });
  };

  return (
    <div className="centered">
      <p>{message}</p>
      <div className="button-group">
        <button className="button" onClick={() => handlePlayAgain("easy")}>
          Play Again (Easy)
        </button>
        <button className="button" onClick={() => handlePlayAgain("medium")}>
          Play Again (Medium)
        </button>
        <button className="button" onClick={() => handlePlayAgain("hard")}>
          Play Again (Hard)
        </button>
      </div>
    </div>
  );
};

export default Results;
