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

  const handlePlayAgain = () => {
    navigate("/play"); // Redirect to the Play component
  };

  return (
    <div className="centered">
      <p>{message}</p>
      <button className="button" onClick={handlePlayAgain}>Play Again</button>
    </div>
  );
};

export default Results;
