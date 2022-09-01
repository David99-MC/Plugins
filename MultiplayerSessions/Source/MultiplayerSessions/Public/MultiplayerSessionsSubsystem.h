// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "MultiplayerSessionsSubsystem.generated.h"

//
// Declaring our own custom delegates for the Menu class to bind callbacks to
// The purpose of declaring these delegates is to avoid DEPENDENCY, which means the Menu class doesn't have to know anything about the Subsystem class
//
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& sessionSearchResult, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessul);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);
/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UMultiplayerSessionsSubsystem();
	//
	// To Handle session functionality. The Menu class will call these
	//
	void CreateSession(int32 NumPublicConnections, FString MatchType);
	void FindSessions(int32 MaxSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	void StartSession();
	void DestroySession();

	//
	// Our own custom delegates for the Menu class to bind callbacks to
	//
	FMultiplayerOnCreateSessionComplete multiplayerOnCreateSessionComplete;
	FMultiplayerOnFindSessionsComplete multiplayerOnFindSessionsComplete;
	FMultiplayerOnJoinSessionComplete multiplayerOnJoinSessionComplete;
	FMultiplayerOnStartSessionComplete multiplayerOnStartSessionComplete;
	FMultiplayerOnDestroySessionComplete multiplayerOnDestroySessionComplete;

protected:
	//
	// Internal callbacks for the delegates we'll add to the Online Session Interface delegate list
	// These don't need to be called outside this class
	//
	void OnCreateSessionComplete(FName sessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result);
	void OnStartSessionComplete(FName sessionName, bool bWasSuccessful);
	void OnDestroySessionComplete(FName sessionName, bool bWasSuccessful);

private:
	IOnlineSessionPtr sessionInterface;
	TSharedPtr<FOnlineSessionSettings> lastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> lastSessionSearch;
	//
	// To add to Online Session Interface delegate list.
	// We'll bind out MultiplayerSessionSubsystem internal callbacks to these.
	//
	FOnCreateSessionCompleteDelegate createSessionCompleteDelegate;
	FDelegateHandle createSessionCompleteDelegateHandle;

	FOnFindSessionsCompleteDelegate findSessionsCompleteDelegate;
	FDelegateHandle findSessionsCompleteDelegateHandle;

	FOnJoinSessionCompleteDelegate joinSessionCompleteDelegate;
	FDelegateHandle joinSessionCompleteDelegateHandle;

	FOnStartSessionCompleteDelegate startSessionCompleteDelegate;
	FDelegateHandle startSessionCompleteDelegateHandle;

	FOnDestroySessionCompleteDelegate destroySessionCompleteDelegate;
	FDelegateHandle destroySessionCompleteDelegateHandle;

	bool bCreateSessionOnDestroy{false};
	int32 lastNumPublicConnections;
	FString lastMatchType;
};
