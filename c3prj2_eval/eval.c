 #include "eval.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int card_ptr_comp(const void *vp1, const void *vp2) {
  const card_t * const * cpp1 = vp1;
  const card_t * const * cpp2 = vp2;

  const card_t * cp1 = *cpp1;
  const card_t * cp2 = *cpp2;

  int retval = 1;
  if (cp1->value > cp2->value) {
    retval = -1;
  }
  else if (cp1->value == cp2->value) {
    if (cp1->suit > cp2->suit) {
      retval = -1;
    }
    else if (cp1->suit < cp2->suit) {
      retval = 1;
    }  // else: suit1 < suit2
  }  // v1 == v2
  else if(cp1->value < cp2->value) {
    retval = 1;
  }
  
  return retval;
}


// checks if hand contains a flush
// return the enum suit_t if found, otherwise NUM_SUITS
suit_t flush_suit(deck_t * hand) {
  // check the suits and count each hand
  size_t nof_per_suit[4] = {0, 0, 0, 0};
  card_t *this_card;
  for (size_t ic = 0; ic < hand->n_cards; ic++) {
    this_card = hand->cards[ic];
    nof_per_suit[this_card->suit]++;
  }  // for: all cards in hand

  // check which suit occurs most ofter
  suit_t retval = NUM_SUITS;
  for (suit_t isuit=SPADES; isuit < NUM_SUITS; isuit++) {
    if (nof_per_suit[isuit] >= 5) {
      retval = isuit;
    }
  }  // for: all suits (cast to intiegers
  return retval;
}

//returns the largest element in an array
unsigned get_largest_element(unsigned * arr, size_t n) {
  unsigned retval = arr[0];
  for (size_t ielm = 1; ielm < n; ielm++) {
    if (arr[ielm] > retval) {
      retval = arr[ielm];
    } // if: element is bigger than retval
  }  // for all elements in array

  //  printf("largest index = %d", retval);
  return retval;
}

size_t get_match_index(unsigned * match_counts, size_t n,unsigned n_of_akind){
  size_t retval = -1;
  for (size_t ielm = 0; ielm < n; ielm++ ) {
    if (match_counts[ielm] == n_of_akind) {
      retval = ielm;
      break;  // break to ensure the lowest index is kept
    }  // if: found n_of_akind
  }  // for: all elements in array
  return retval;
}

// check if there is a secondary pair (only a pair is to be found)
ssize_t find_secondary_pair(deck_t * hand,
	                    unsigned * match_counts,
			    size_t match_idx) {
  // init return value
  ssize_t retval = -1;

  for (size_t ielm = match_idx+2; ielm < hand->n_cards; ielm++) {
    if (match_counts[ielm] == 2) {
      retval = ielm;
      break;
    }  // if: a secondary pair is found
  }  // for: counting from previously found index

  return retval;
}

// if there is a straight (flush) at an index to test *index*
int is_n_length_straight_at(deck_t * hand, size_t index, suit_t fs, size_t n) {
  
  // check suit and value of index
  card_t card_at_index = *(hand->cards[index]);  // double pointer
  unsigned value_at_index = card_at_index.value;

  
  unsigned is_fnd = 1;  // pseudo boolean
  // loop all indices from index to end
  for (size_t i=1; i<n; i++) {
    // value to find
    unsigned val2find = value_at_index - i;
    is_fnd = 0;
    for (size_t j=index+1; j<hand->n_cards; j++) {
      if (hand->cards[j]->value == val2find) {
	if (fs == NUM_SUITS || hand->cards[j]->suit == fs) {
	  is_fnd = 1;
	} // incorrect suit for straight flush
      }  // if: correct value isfound
    }  // for: looping all cards in the deck

    if (is_fnd == 0) {
      break;
    }  // if: missing card for a straight
  } // for: counting from 1 to 5

  return is_fnd;
}

// check if there are n cards in a row
int is_ace_low_straight_at(deck_t *hand, size_t index, suit_t fs) {
  // find low straight
  int is_ace_fnd = 0;
  int low_part_fnd = is_n_length_straight_at(hand, index, fs, 4);
  if (low_part_fnd == 1) {
    // check if ace is present
    for (size_t i=index+1; i < 4; i++) {
      if (hand->cards[i]->suit == fs || fs == NUM_SUITS) {
	is_ace_fnd = -1;
      }  // if wanted ace found
    }  // loop first 4 elements (ordered, thus only 4 aces possible)
  }  // if: 5, 4, 3, 2 sequence found

  return is_ace_fnd;
}

int is_straight_at(deck_t *hand, size_t index, suit_t fs) {
  // check for generic straight
  int retval = is_n_length_straight_at(hand, index, fs, 5);
  if (retval == 0) {
    retval = is_ace_low_straight_at(hand, index, fs);
  }  // if: no generic straight found
  
  return retval;
}


hand_eval_t build_hand_from_match(deck_t * hand,
				  unsigned n,
				  hand_ranking_t what,
				  size_t idx) {
  
  hand_eval_t ans;

  // set ranking
  ans.ranking = what;

  size_t counter = 0;
  for (size_t icard = idx; icard < idx+n; icard++) {
    ans.cards[counter] = hand->cards[icard];
    counter++;  
  }  // for: n cards starting from idx

  if (counter < 5) {
    size_t nof_before = idx;
    if (5 - counter < idx) {
      nof_before = 5 - counter;
    }  // upto the amount still to do
    for (size_t ibefore=0; ibefore < nof_before; ibefore++) {
      ans.cards[counter] = hand->cards[ibefore];
      counter++;
    }  // for: all before

    if (counter < 5) {
      size_t nof_after = 5 - counter;
      for (size_t iafter=0; iafter < nof_after; iafter++) {
	ans.cards[counter] = hand->cards[idx+n+iafter];
	counter++;
      }  // for: all after
    }  // if: still some cards to pick
  } // more cards to add

  return ans;
}


int compare_hands(deck_t * hand1, deck_t * hand2) {
  int retval = 0;

  // sort each hand
  qsort(hand1->cards, hand1->n_cards, sizeof(hand1->cards), card_ptr_comp);
  qsort(hand2->cards, hand2->n_cards, sizeof(hand2->cards[0]), card_ptr_comp);

  hand_eval_t heval1 = evaluate_hand(hand1);
  hand_eval_t heval2 = evaluate_hand(hand2);

  if (heval1.ranking == heval2.ranking) {
    // is a tie
    for (size_t icard = 0; icard < 5; icard++) {
      retval = heval1.cards[icard]->value -
	       heval2.cards[icard]->value;
      // if there is a difference, break and keep retval
      if (retval != 0) {
	break;
      }  // if difference found
    }  // loop all 5 cards
  }
  else {
    if (heval1.ranking > heval2.ranking) {
      retval = 1;
    }  // if hand 1 is better
    else {
      retval = -1;
    }  // hand 2 is better
  }  // no tie
  return retval;
}



//You will write this function in Course 4.
//For now, we leave a prototype (and provide our
//implementation in eval-c4.o) so that the
//other functions we have provided can make
//use of get_match_counts.
unsigned * get_match_counts(deck_t * hand) ;

// We provide the below functions.  You do NOT need to modify them
// In fact, you should not modify them!


//This function copies a straight starting at index "ind" from deck "from".
//This copies "count" cards (typically 5).
//into the card array "to"
//if "fs" is NUM_SUITS, then suits are ignored.
//if "fs" is any other value, a straight flush (of that suit) is copied.
void copy_straight(card_t ** to, deck_t *from, size_t ind, suit_t fs, size_t count) {
  assert(fs == NUM_SUITS || from->cards[ind]->suit == fs);
  unsigned nextv = from->cards[ind]->value;
  size_t to_ind = 0;
  while (count > 0) {
    assert(ind < from->n_cards);
    assert(nextv >= 2);
    assert(to_ind <5);
    if (from->cards[ind]->value == nextv &&
	(fs == NUM_SUITS || from->cards[ind]->suit == fs)){
      to[to_ind] = from->cards[ind];
      to_ind++;
      count--;
      nextv--;
    }
    ind++;
  }
}


//This looks for a straight (or straight flush if "fs" is not NUM_SUITS)
// in "hand".  It calls the student's is_straight_at for each possible
// index to do the work of detecting the straight.
// If one is found, copy_straight is used to copy the cards into
// "ans".
int find_straight(deck_t * hand, suit_t fs, hand_eval_t * ans) {
  if (hand->n_cards < 5){
    return 0;
  }
  for(size_t i = 0; i <= hand->n_cards -5; i++) {
    int x = is_straight_at(hand, i, fs);
    if (x != 0){
      if (x < 0) { //ace low straight
	assert(hand->cards[i]->value == VALUE_ACE &&
	       (fs == NUM_SUITS || hand->cards[i]->suit == fs));
	ans->cards[4] = hand->cards[i];
	size_t cpind = i+1;
	while(hand->cards[cpind]->value != 5 ||
	      !(fs==NUM_SUITS || hand->cards[cpind]->suit ==fs)){
	  cpind++;
	  assert(cpind < hand->n_cards);
	}
	copy_straight(ans->cards, hand, cpind, fs,4) ;
      }
      else {
	copy_straight(ans->cards, hand, i, fs,5);
      }
      return 1;
    }
  }
  return 0;
}


//This function puts all the hand evaluation logic together.
//This function is longer than we generally like to make functions,
//and is thus not so great for readability :(
hand_eval_t evaluate_hand(deck_t * hand) {
  suit_t fs = flush_suit(hand);
  hand_eval_t ans;
  if (fs != NUM_SUITS) {
    if(find_straight(hand, fs, &ans)) {
      ans.ranking = STRAIGHT_FLUSH;
      return ans;
    }
  }
  unsigned * match_counts = get_match_counts(hand);
  unsigned n_of_a_kind = get_largest_element(match_counts, hand->n_cards);
  assert(n_of_a_kind <= 4);
  size_t match_idx = get_match_index(match_counts, hand->n_cards, n_of_a_kind);
  ssize_t other_pair_idx = find_secondary_pair(hand, match_counts, match_idx);
  free(match_counts);
  if (n_of_a_kind == 4) { //4 of a kind
    return build_hand_from_match(hand, 4, FOUR_OF_A_KIND, match_idx);
  }
  else if (n_of_a_kind == 3 && other_pair_idx >= 0) {     //full house
    ans = build_hand_from_match(hand, 3, FULL_HOUSE, match_idx);
    ans.cards[3] = hand->cards[other_pair_idx];
    ans.cards[4] = hand->cards[other_pair_idx+1];
    return ans;
  }
  else if(fs != NUM_SUITS) { //flush
    ans.ranking = FLUSH;
    size_t copy_idx = 0;
    for(size_t i = 0; i < hand->n_cards;i++) {
      if (hand->cards[i]->suit == fs){
	ans.cards[copy_idx] = hand->cards[i];
	copy_idx++;
	if (copy_idx >=5){
	  break;
	}
      }
    }
    return ans;
  }
  else if(find_straight(hand,NUM_SUITS, &ans)) {     //straight
    ans.ranking = STRAIGHT;
    return ans;
  }
  else if (n_of_a_kind == 3) { //3 of a kind
    return build_hand_from_match(hand, 3, THREE_OF_A_KIND, match_idx);
  }
  else if (other_pair_idx >=0) {     //two pair
    assert(n_of_a_kind ==2);
    ans = build_hand_from_match(hand, 2, TWO_PAIR, match_idx);
    ans.cards[2] = hand->cards[other_pair_idx];
    ans.cards[3] = hand->cards[other_pair_idx + 1];
    if (match_idx > 0) {
      ans.cards[4] = hand->cards[0];
    }
    else if (other_pair_idx > 2) {  //if match_idx is 0, first pair is in 01
      //if other_pair_idx > 2, then, e.g. A A K Q Q
      ans.cards[4] = hand->cards[2];
    }
    else {       //e.g., A A K K Q
      ans.cards[4] = hand->cards[4];
    }
    return ans;
  }
  else if (n_of_a_kind == 2) {
    return build_hand_from_match(hand, 2, PAIR, match_idx);
  }
  return build_hand_from_match(hand, 0, NOTHING, 0);
}