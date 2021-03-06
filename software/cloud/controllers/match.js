const User = require('../models/user');
const Match = require('../models/match');

const updateUserRecord = async (email, didWinGame) => {
    if (didWinGame) {
        // Increment wins by 1
        await User.findOneAndUpdate({ email: email }, { $inc: { "wins": 1 }});
    }
    else {
        // Increment losses by 1
        await User.findOneAndUpdate({ email: email }, { $inc: { "losses": 1 }});
    }
}

module.exports = {
	getMatches: async (req, res) => {
		const userEmail = res.locals.email

        // Check if user exists
        const myUser = await User.findOne({ email: userEmail }).catch(() => null);

        // If User exists, get an array of that user's matches
        if (myUser) {
            const myMatchesP1 = await Match.find({ player_one: userEmail }).catch(() => []);
            const myMatchesP2 = await Match.find({ player_two: userEmail }).catch(() => []);
            const allMyMatches = myMatchesP1.concat(myMatchesP2);
            
            // Sort matches by most recent date
            allMyMatches.sort((a, b) => parseFloat(b.date) - parseFloat(a.date));
            
            res.status(200).send(
                {
                    "status" : "OK",
                    "matches" : allMyMatches,
                }
            )
        }
        // If not, respond with a 404
        else {
            res.status(404).send({"status" : "ERR"});
		}
	},
    
    createMatch: async (req, res) => {
        // Check if player1 exists
        const playerOneId = req.body.player_one;
        
        const player1 = await User.findOne({ player_id: playerOneId }).catch(() => null);
        if (player1) {
            
            const playerOneEmail = player1.email;
            // If player two is an AI
            if (req.body.player_two === 0 || req.body.player_two === 1) {
                // Create a record of the match
                let nameOfAI = (req.body.player_two === 0) ? "Easy AI" : "Hard AI";
                let emailOfAI = (req.body.player_two === 0) ? "Easy_AI_Email" : "Hard_AI_Email";
                let winnerEmail = (req.body.winner == playerOneId) ? playerOneEmail : emailOfAI;
                
                const newMatch = {
                    player_one: playerOneEmail,
                    player_two: nameOfAI,
                    winner: winnerEmail,
                    player_one_score: req.body.player_one_score,
                    player_two_score: req.body.player_two_score,
                    player_one_name: `${player1.first_name} ${player1.last_name}`,
                    player_two_name: nameOfAI,
                    date: Date.now() * 1000
                }
                await Match.create(newMatch);
                
                // Update only player one's personal stats
                await updateUserRecord(playerOneEmail, (playerOneEmail === winnerEmail));
                
                // Respond with a 201
                res.status(201).send(newMatch);
            }
            // If player two is NOT an AI
            else {
                // Verify that player two exists as a user
                const player2 = await User.findOne({ player_id: req.body.player_two }).catch(() => null);
                if (player2) {
                    const playerTwoEmail = player2.email;
                    const winnerEmail = (req.body.winner == playerOneId) ? playerOneEmail : playerTwoEmail;
                    // Create a record of the match
                    const newMatch = {
                        player_one: playerOneEmail,
                        player_two: playerTwoEmail,
                        winner: winnerEmail,
                        player_one_score: req.body.player_one_score,
                        player_two_score: req.body.player_two_score,
                        player_one_name: `${player1.first_name} ${player1.last_name}`,
                        player_two_name: `${player2.first_name} ${player2.last_name}`,
                        date: Date.now() * 1000
                    }
                    await Match.create(newMatch);
                    
                    // Update both players' personal stats
                    await updateUserRecord(playerOneEmail, (playerOneEmail === winnerEmail));
                    await updateUserRecord(playerTwoEmail, (playerTwoEmail === winnerEmail));
                    
                    // Respond with a 201
                    res.status(201).send(newMatch);
                }
                // Respond with an error if player two doesn't exist
                else {
                    res.status(404).send(
                        {
                            "status" : "ERR",
                            "message" : "No user has id" + req.body.player_two.toString()
                        }
                    );
                }
            }
        }
        // Respond with an error if player one doesn't exist
        else {
            res.status(404).send(
                {
                    "status" : "ERR",
                    "message" : "No user has id" + req.body.player_one.toString()
                }
            );
		}
    }
}